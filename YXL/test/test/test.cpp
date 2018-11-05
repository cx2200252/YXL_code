#include "test.h"

namespace YXL_TEST
{
	using YXL::CStr;

	void SaveTestFile(const std::string& fn, const char* data, const size_t size)
	{
		auto test_dir = GetTestDir();
		std::ofstream fout(test_dir+fn, std::ios::binary);
		fout.write(data, size);
		fout.close();
	}

	void GetTestFile(const std::string& fn, std::string& content)
	{
		auto test_dir = GetTestDir();
		YXL::File::LoadFileContentBinary(test_dir + fn, content);
	}

	void PrintKey(CStr& key)
	{
		for (int i(0); i != key.length(); ++i)
		{
			if (i)
				std::cout << ",";
			std::cout << (unsigned int)(unsigned char)key[i] << std::endl;
		}
	}

	void TestZip()
	{
		std::cout << "*********************************************************************************" << std::endl;
#ifndef _YXL_MINI_Z_
		std::cout << "miniz not enable..." << std::endl;
#else
		{
			std::cout << "*********test create zip*********" << std::endl;
			YXL::ZIP::Zip zip;
			zip.AddFile("1.txt", "1\nabc abc\n 3.14");
			zip.AddFile("2.txt", "test create");
			std::shared_ptr<char> ptr;
			size_t size;
			bool ret = zip.ToZip(ptr, size, YXL::ZIP::ZIP_COMPRESSION::LEVEL_3);
			if (ret)
				SaveTestFile("test_zip_1.zip", ptr.get(), size);
		}
		{
			std::cout << "*********load & add with unzip*********" << std::endl;
			std::string zip_content;
			GetTestFile("test_zip_1.zip", zip_content);
			YXL::ZIP::Zip zip(zip_content, true);
			std::map<std::string, std::shared_ptr<YXL::ZIP::File>> files;
			zip.GetFiles(files);
			if (false == files.empty())
			{
				std::cout << "files in zip: " << std::endl;
				for (auto pair : files)
					std::cout << "\t" << pair.first << std::endl;
			}
			else
			{
				std::cout << "no file in zip or not unzip..." << std::endl;
			}

			zip.AddFile("3.txt", "123456");

			std::cout << "*********test file reading*********" << std::endl;
			auto f = files["1.txt"];
			std::cout << "int: " << f->ReadInt() << std::endl;
			std::cout << "char: " << f->ReadChar() << std::endl;
			std::cout << "string: " << f->ReadString() << std::endl;
			std::cout << "string: " << f->ReadString() << std::endl;
			std::cout << "double: " << f->ReadDouble() << std::endl;

			std::shared_ptr<char> ptr;
			size_t size;
			bool ret = zip.ToZip(ptr, size);
			if(ret)
				SaveTestFile("test_zip_2.zip", ptr.get(), size);
		}
		{
			std::cout << "*********load & add without unzip*********" << std::endl;
			std::string zip_content;
			GetTestFile("test_zip_1.zip", zip_content);
			YXL::ZIP::Zip zip(zip_content, false);
			std::map<std::string, std::string> files;
			zip.GetFiles(files);
			if (false == files.empty())
			{
				std::cout << "files in zip: " << std::endl;
				for (auto pair : files)
					std::cout << "\t" << pair.first << std::endl;
			}
			else
			{
				std::cout << "no file in zip or not unzip yet..." << std::endl;
			}

			zip.AddFile("3.txt", "123456");
			std::shared_ptr<char> ptr;
			size_t size;
			bool ret = zip.ToZip(ptr, size);
			if (ret)
				SaveTestFile("test_zip_3.zip", ptr.get(), size);
		}
#endif
	}

	void TestEncryptedData()
	{
		std::cout << "*********************************************************************************" << std::endl;
#ifndef _YXL_ENCRYPT_DATA_
		std::cout << "encrypted data not enable..." << std::endl;
#else
		typedef std::pair<std::string, std::string> KeyPair;

		{
			std::cout << "*********test default signer*********" << std::endl;
			CStr test = "abcdef123456789";
			KeyPair a, b;
			YXL::Crypt::DefSigner::GenKeyPair(a.first, a.second);
			YXL::Crypt::DefSigner::GenKeyPair(b.first, b.second);
			printf("pk len(in byte): %d\nsk len(in byte): %d\n", a.first.length(), a.second.length());

			YXL::Crypt::DefSigner signA(a.second, b.first);
			YXL::Crypt::DefSigner signB(b.second, a.first);

			std::string ret;
			signA.Sign(ret, test.c_str(), test.length());
			printf("src text len(in byte): %d\nsigned text len(in byte): %d\n", test.length(), ret.length());

			std::string ret2;
			auto flag = signB.Verify(ret2, ret.c_str(), ret.length());
			std::cout << "verify result: " << flag << std::endl;
		}
		{
			std::cout << "*********test default sys encryptor*********" << std::endl;
			std::string key;
			YXL::Crypt::DefSymEncryptor::GenKey(key);
			printf("key len(in byte): %d\n", key.length());

			CStr test = "abcdef123456789";
			CStr nonce = "";

			YXL::Crypt::DefSymEncryptor encryptor(key);
			std::string ret;
			encryptor.Encrypt(ret, test.c_str(), test.length(), nonce);
			std::cout << "src text: " << test << std::endl;
			printf("src text len(in byte): %d\nencrypted text len(in byte): %d\n", test.length(), ret.length());

			std::string ret2;
			encryptor.Decrypt(ret2, ret.c_str(), ret.length(), nonce);
			std::cout << "decrypted text: " << ret2 << std::endl;
		}
		{
			std::cout << "*********test default asym encryptor*********" << std::endl;
			KeyPair a, b;
			YXL::Crypt::DefAsymEncryptor::GenKeyPair(a.first, a.second);
			YXL::Crypt::DefAsymEncryptor::GenKeyPair(b.first, b.second);

			printf("pk len(in byte): %d\nsk len(in byte): %d\n", a.first.length(), a.second.length());

			CStr test = "abcdef123456789";
			CStr nonce = "";

			YXL::Crypt::DefAsymEncryptor encryptorA("", b.first);
			YXL::Crypt::DefAsymEncryptor encryptorB(b.second, "");

			std::string ret;
			encryptorA.Encrypt(ret, test.c_str(), test.length(), nonce);
			std::cout << "src text: " << test << std::endl;
			printf("src text len(in byte): %d\nencrypted text len(in byte): %d\n", test.length(), ret.length());

			std::string ret2;
			encryptorB.Decrypt(ret2, ret.c_str(), ret.length(), nonce);
			std::cout << "decrypted text: " << ret2 << std::endl;
		}
		{
			std::cout << "*********test EncryptedData*********" << std::endl;
			KeyPair key;
			YXL::Crypt::DefAsymEncryptor::GenKeyPair(key.first, key.second);
			std::string key2;
			YXL::Crypt::DefSymEncryptor::GenKey(key2);

			CStr test = "abcdef123456789";
			CStr nonce = "";

			std::string ret;
			YXL::Crypt::DefEncryptedData::Pack(ret, test, key.first, key2, nonce);
			std::string ret2;
			YXL::Crypt::DefEncryptedData::Unpack(ret2, ret, key.second, "");
			std::cout << "src text: " << test << std::endl;
			std::cout << "decrypted text: " << ret2 << std::endl;
		}
#endif
	}

}