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
			std::cout << (unsigned int)(unsigned char)key[i];
		}
		std::cout << std::endl;
	}

#ifdef _YXL_LOG_
	void Log(std::shared_ptr<YXL::Logger> logger, int level, int& idx)
	{
		if (level == 3)
			return;
		for (int i(0); i != 3; ++i)
		{
			logger->Down();
			Log(logger, level + 1, idx);
			logger->Up();
			logger->Log("level %d: %d", level, idx++);
		}
	}
#endif

	void TestLog()
	{
		std::cout << "*********************************************************************************" << std::endl;
#ifndef _YXL_LOG_
		std::cout << "log not enable..." << std::endl;
#else
		std::shared_ptr<YXL::Logger> logger(new YXL::Logger);
		int idx = 0;
		Log(logger, 0, idx);
		logger->Log("level %d: %d", 0, idx++);

		std::cout << "---------" << std::endl;
		std::string str;
		logger->GetLog(str);
		std::cout << str;
		std::cout << "---------" << std::endl;
#endif
	}

	void TestZip()
	{
		std::cout << "*********************************************************************************" << std::endl;
#ifndef _YXL_COMPRESS_MINI_Z_
		std::cout << "miniz not enable..." << std::endl;
#else
		{
			std::cout << "---------test create zip---------" << std::endl;
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
			std::cout << "---------load & add with unzip---------" << std::endl;
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

			std::cout << "---------test file reading---------" << std::endl;
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
			std::cout << "---------load & add without unzip---------" << std::endl;
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

	void TestNaCL()
	{
		std::cout << "*********************************************************************************" << std::endl;
#ifndef _YXL_CRYPTOGRAPHIC_NACL_
		std::cout << "NaCL not enable..." << std::endl;
#else
		typedef std::pair<std::string, std::string> KeyPair;

		{
			std::cout << "---------test signer---------" << std::endl;
			CStr test = "abcdef123456789";
			KeyPair a, b;
			YXL::Crypt::SignerNaCL::GenKeyPair(a.first, a.second);
			YXL::Crypt::SignerNaCL::GenKeyPair(b.first, b.second);
			printf("pk len(in byte): %d\nsk len(in byte): %d\n", (int)a.first.length(), (int)a.second.length());

			YXL::Crypt::SignerNaCL signA(a.second, b.first);
			YXL::Crypt::SignerNaCL signB(b.second, a.first);

			std::string ret;
			signA.Sign(ret, test.c_str(), (int)test.length());
			printf("src text len(in byte): %d\nsigned text len(in byte): %d\n", (int)test.length(), (int)ret.length());

			auto flag = signB.Verify(ret, test.c_str(), (int)test.length());
			std::cout << "verify result: " << (flag?"true":"false") << std::endl;
		}
		{
			std::cout << "---------test sys encryptor---------" << std::endl;
			std::string key;
			YXL::Crypt::SymEncryptorNaCL::GenKey(key);
			printf("key len(in byte): %d\n", (int)key.length());

			CStr test = "abcdef123456789";
			CStr nonce = "";

			YXL::Crypt::SymEncryptorNaCL encryptor(key);
			std::string ret;
			encryptor.Encrypt(ret, test.c_str(), (int)test.length(), nonce);
			std::cout << "src text: " << test << std::endl;
			printf("src text len(in byte): %d\nencrypted text len(in byte): %d\n", (int)test.length(), (int)ret.length());

			std::string ret2;
			encryptor.Decrypt(ret2, ret.c_str(), (int)ret.length(), nonce);
			std::cout << "decrypted text: " << ret2 << std::endl;
		}
		{
			std::cout << "---------test asym encryptor---------" << std::endl;
			KeyPair a, b;
			YXL::Crypt::AsymEncryptorNaCL::GenKeyPair(a.first, a.second);
			YXL::Crypt::AsymEncryptorNaCL::GenKeyPair(b.first, b.second);

			printf("pk len(in byte): %d\nsk len(in byte): %d\n", (int)a.first.length(), (int)a.second.length());

			CStr test = "abcdef123456789";
			CStr nonce = "";

			YXL::Crypt::AsymEncryptorNaCL encryptorA("", b.first);
			YXL::Crypt::AsymEncryptorNaCL encryptorB(b.second, "");

			std::string ret;
			encryptorA.Encrypt(ret, test.c_str(), (int)test.length(), nonce);
			std::cout << "src text: " << test << std::endl;
			printf("src text len(in byte): %d\nencrypted text len(in byte): %d\n", (int)test.length(), (int)ret.length());

			std::string ret2;
			encryptorB.Decrypt(ret2, ret.c_str(), (int)ret.length(), nonce);
			std::cout << "decrypted text: " << ret2 << std::endl;
		}
		{
			std::cout << "---------test EncryptedData---------" << std::endl;
			KeyPair key;
			YXL::Crypt::AsymEncryptorNaCL::GenKeyPair(key.first, key.second);
			std::string key2;
			YXL::Crypt::SymEncryptorNaCL::GenKey(key2);

			CStr test = "abcdef123456789";
			CStr nonce = "";

			std::string ret;
			YXL::Crypt::EncryptedDataNaCL::Pack(ret, test.c_str(), test.size(), key.first, key2, nonce);
			std::string ret2;
			YXL::Crypt::EncryptedDataNaCL::Unpack(ret2, ret.c_str(), ret.size(), key.second, nonce);
			std::cout << "src text: " << test << std::endl;
			std::cout << "decrypted text: " << ret2 << std::endl;
		}
		{
			std::cout << "---------test DataContainer---------" << std::endl;
			CStr test_zip = GetTestDir() + "test_zip_3.zip";
			if (false == YXL::File::FileExist(test_zip))
			{
				std::cout << "test zip not exist: " << test_zip << std::endl;
			}
			else
			{
				KeyPair key;
				YXL::Crypt::AsymEncryptorNaCL::GenKeyPair(key.first, key.second);
				std::string key2;
				YXL::Crypt::SymEncryptorNaCL::GenKey(key2);

				std::string zip;
				YXL::File::LoadFileContentBinary(test_zip, zip);
				CStr nonce = "";

				std::string ret;
				YXL::Crypt::EncryptedDataNaCL::Pack(ret, zip.c_str(), zip.size(), key.first, key2, nonce);

				YXL::Crypt::DataContainerNaCL data(key.second);
				data.Load(ret.c_str(), ret.size(), nonce);

				std::cout << "1.txt exist: " << (data.FileExist("1.txt") ? "true" : "false") << std::endl;
				std::cout << "4.txt exist: " << (data.FileExist("4.txt") ? "true" : "false") << std::endl;

				auto f = data.GetFile("1.txt");
				std::cout << "int: " << f->ReadInt() << std::endl;
				std::cout << "char: " << f->ReadChar() << std::endl;
				std::cout << "string: " << f->ReadString() << std::endl;
				std::cout << "string: " << f->ReadString() << std::endl;
				std::cout << "double: " << f->ReadDouble() << std::endl;
			}
			
		}
#endif
	}

	void TestCrypto()
	{
		std::cout << "*********************************************************************************" << std::endl;
#ifndef _YXL_CRYPTOGRAPHIC_CRYPTO_
		std::cout << "Cryptopp not enable..." << std::endl;
#else
		{
			std::cout << "---------test hash---------" << std::endl;

			CStr test = "abcdef123456789";

#define Item(name) {#name, std::shared_ptr<YXL::Crypt::HasherCryptoPPBase > (new YXL::Crypt::Hasher##name)}
			std::vector<std::pair<std::string, std::shared_ptr<YXL::Crypt::HasherCryptoPPBase>>> hasher = {
				Item(MD5),
				Item(SHA1),
				Item(SHA224),
				Item(SHA384),
				Item(SHA512),
				Item(SHA3_256),
				Item(SHA3_384),
				Item(SHA3_512),
				Item(Tiger),
				Item(RIPEMD128),
				Item(RIPEMD160),
				Item(Whirlpool),
			};
#undef Item

			std::cout << "hash string:" << std::endl;

			for (auto h : hasher)
			{
				std::string out;
				h.second->Hash(out, test.c_str(), test.length());
				std::cout << "\t" << h.first << ": " << out << std::endl;
			}

			std::cout << "hash file:" << std::endl;
			SaveTestFile("hash.txt", test.c_str(), test.length());

			for (auto h : hasher)
			{
				std::string out;
				h.second->HashFile(out, GetTestDir() + "hash.txt");
				std::cout << "\t" << h.first << ": " << out << std::endl;
			}

		}

		{
			std::cout << "---------test signer---------" << std::endl;

			CStr test = "abcdef123456789";

			std::pair<std::string, std::string> key_a, key_b;
			std::string seed = YXL::Crypt::SignerRSA::GenSeed(1024);
			YXL::Crypt::SignerRSA::GenKeyPair(key_a.first, key_a.second, seed, 512);
			YXL::Crypt::SignerRSA::GenKeyPair(key_b.first, key_b.second, seed, 512);
			printf("pk len(in byte): %d\nsk len(in byte): %d\n", (int)key_a.first.length(), (int)key_a.second.length());

			YXL::Crypt::SignerRSA signer_a(key_a.second, key_b.first);
			YXL::Crypt::SignerRSA signer_b(key_b.second, key_a.first);
			{
				std::cout << "sign string:" << std::endl;
				std::string ret;
				signer_a.Sign(ret, test.c_str(), test.length());
				printf("\tsrc text len(in byte): %d\n\tsignature len(in byte): %d\n", (int)test.length(), (int)ret.length());

				std::string ret2;
				bool flag = signer_b.Verify(ret, test.c_str(), test.length());
				std::cout << "\tverify result: " << (flag ? "true" : "false") << std::endl;

				flag = signer_a.Verify(ret, test.c_str(), test.length());
				std::cout << "\tverify result: " << (flag ? "true" : "false") << std::endl;
			}
			{
				std::cout << "sign file:" << std::endl;

				SaveTestFile("test_sign.txt", test.c_str(), test.length());

				std::string ret;
				signer_a.SignFile(ret, GetTestDir()+"test_sign.txt");
				printf("\tsrc text len(in byte): %d\n\tsignature len(in byte): %d\n", (int)test.length(), (int)ret.length());

				std::string ret2;
				bool flag = signer_b.VerifyFile(ret, GetTestDir() + "test_sign.txt");
				std::cout << "\tverify result: " << (flag ? "true" : "false") << std::endl;

				flag = signer_a.VerifyFile(ret, GetTestDir() + "test_sign.txt");
				std::cout << "\tverify result: " << (flag ? "true" : "false") << std::endl;
			}
			

		}

		{
			std::cout << "---------test AES---------" << std::endl;

			CStr test = "abcdef123456789";
			const int key_len = 32;//aes256
			CStr nonce = YXL::Crypt::EncryptorBase::GenRandomBuffer(key_len);
			SaveTestFile("test_aes.txt", test.c_str(), test.length());
			CStr fn_test = GetTestDir() + "test_aes.txt";

#define _AES(mode) \
			{\
				std::cout << "mode: " << #mode << std::endl;\
				auto key = YXL::Crypt::AES_##mode::GenKey(key_len);\
				printf("key len(in byte): %d\n", (int)key.length());\
				YXL::Crypt::AES_CBC aes(key);\
				{\
					std::cout << "encrypt string:" << std::endl;\
					std::string cipher;\
					aes.Encrypt(cipher, test.c_str(), test.length(), nonce);\
					printf("\tplain text len(in byte): %d\n\tcipher len(in byte): %d\n", (int)test.length(), (int)cipher.length());\
					std::string ret;\
					aes.Decrypt(ret, cipher.c_str(), cipher.length(), nonce);\
					printf("\tsrc text: %s\n\tdecrypted text: %s\n", test.c_str(), ret.c_str());\
				}\
				{\
					std::cout << "encrypt file:" << std::endl;\
					CStr enc = GetTestDir() + "aes_" + #mode + "_encrypt.bin";\
					CStr dec = GetTestDir() + "aes_" + #mode + "_decrypt.txt";\
					aes.Encrypt_File(enc, fn_test, nonce);\
					std::cout << "\tcipher file: " << enc << std::endl;\
					aes.Decrypt_File(dec, enc, nonce);\
					std::cout << "\tdecrpyted file: " << dec << std::endl;\
				}\
			}
			_AES(CFB);
			_AES(OFB);
			_AES(CTR);
			_AES(ECB);
			_AES(CBC);
#undef _AES
		}

		{
			std::cout << "---------test RSA---------" << std::endl;

			CStr test = "abcdef123456789";
			const int key_len = 512;
			CStr nonce = "";
			SaveTestFile("test_rsa.txt", test.c_str(), test.length());
			CStr fn_test = GetTestDir() + "test_rsa.txt";

			std::pair<std::string, std::string> key_a, key_b;
			std::string seed = YXL::Crypt::RSA::GenSeed(1024);
			YXL::Crypt::RSA::GenKeyPair(key_a.first, key_a.second, seed, key_len);
			YXL::Crypt::RSA::GenKeyPair(key_b.first, key_b.second, seed, key_len);
			printf("pk len(in byte): %d\nsk len(in byte): %d\n", (int)key_a.first.length(), (int)key_a.second.length());

			YXL::Crypt::RSA encryptor(key_b.second, key_a.first);
			YXL::Crypt::RSA decryptor(key_a.second, key_b.first);

			{
				std::cout << "encrypt string:" << std::endl;
				std::string ret;
				encryptor.Encrypt(ret, test.c_str(), test.length(), nonce);
				printf("\tsrc text len(in byte): %d\n\tcipher len(in byte): %d\n", (int)test.length(), (int)ret.length());

				std::string ret2;
				decryptor.Decrypt(ret2, ret.c_str(), ret.length(), nonce);
				printf("\tsrc text: %s\n\tdecrypted text: %s\n", test.c_str(), ret2.c_str());

				std::string ret3;
				encryptor.Decrypt(ret3, ret.c_str(), ret.length(), nonce);
				printf("\tsrc text: %s\n\tdecrypted text: %s\n", test.c_str(), ret3.c_str());
			}
			{
				std::cout << "encrypt file:" << std::endl;

				CStr enc = GetTestDir() + "rsa_encrypt.bin"; 
				CStr dec = GetTestDir() + "rsa_decrypt.txt"; 

				encryptor.Encrypt_File(enc, fn_test, nonce);
				std::cout << "\tcipher file: " << enc << std::endl;
				decryptor.Decrypt_File(dec, enc, nonce);
				std::cout << "\tdecrpyted file: " << dec << std::endl;
			}
		}
#endif
	}

	void TestImage()
	{
		std::cout << "*********************************************************************************" << std::endl;
#ifndef _YXL_IMG_CODEC_
		std::cout << "image codec not enable..." << std::endl;
#else
		std::string test_img, test_img2;
		YXL::File::LoadFileContentBinary(GetTestDir() + "test.png", test_img);
		YXL::File::LoadFileContentBinary(GetTestDir() + "test.webp", test_img2);

		int test_pix_x = 230;
		int test_pix_y = 70;
		std::cout << "pixel at (" << test_pix_x << "," << test_pix_y << ") in RGB order: (200,70,70)" << std::endl;;

#ifdef _YXL_IMG_CODEC_STB_IMAGE_
		std::cout << "---------test png codec---------" << std::endl;
#define _TEST(in, out, ch)\
		{\
			std::shared_ptr<unsigned char> data = nullptr;\
			int w(0), h(0);\
			YXL::Image::DecodePNG_##in(data, w, h, test_img);\
			std::shared_ptr<unsigned char> data2 = nullptr;\
			int size(0);\
			YXL::Image::EncodePNG_##out(data2, size, data.get(), w, h);\
			YXL::File::WriteFileContentBinary(GetTestDir() + "test_i"+#in+"_o"+#out+".png", std::string((char*)data2.get(), size));\
			std::cout<<"in("<<#in<<"):\t";\
			std::cout << "(" << (int)data.get()[test_pix_y*w*ch + test_pix_x*ch] << "," << (int)data.get()[test_pix_y*w*ch + test_pix_x*ch + 1] << "," << (int)data.get()[test_pix_y*w*ch + test_pix_x*ch + 2] <<")"<< std::endl;\
		}
		_TEST(RGB, RGB, 3);
		_TEST(BGR, RGB, 3);
		_TEST(RGBA, RGBA, 4);
		_TEST(BGRA, RGBA, 4);
#undef _TEST
#endif

#ifdef _YXL_IMG_CODEC_WEBP_
		std::cout << "---------test webp codec---------" << std::endl;
#define _TEST(in, out, ch)\
		{\
			std::shared_ptr<unsigned char> data = nullptr; \
			int w(0), h(0); \
			YXL::Image::DecodeWebP_##in(data, w, h, test_img2); \
			std::shared_ptr<unsigned char> data2 = nullptr;\
			int size(0);\
			YXL::Image::EncodeWebP_##out(data2, size, data.get(), w, h, 100);\
			YXL::File::WriteFileContentBinary(GetTestDir() + "test_i"+#in+"_o"+#out+".webp", std::string((char*)data2.get(), size));\
			std::cout<<"in("<<#in<<"):\t";\
			std::cout << "(" << (int)data.get()[test_pix_y*w*ch + test_pix_x*ch] << "," << (int)data.get()[test_pix_y*w*ch + test_pix_x*ch + 1] << "," << (int)data.get()[test_pix_y*w*ch + test_pix_x*ch + 2] <<")"<< std::endl;\
		}
		_TEST(RGB, RGB, 3);
		_TEST(BGR, RGB, 3);
		_TEST(RGBA, RGBA, 4);
		_TEST(BGRA, RGBA, 4);
#undef _TEST
#endif

#endif
	}

	void TestSemaphore()
	{
		std::shared_ptr<YXL::Semaphore> semaphore = nullptr;

		auto func = [&]() {
			std::cout << YXL::GetCurrentThreadID() << "\twaiting..." << std::endl;
			semaphore->Wait();
			std::cout << YXL::GetCurrentThreadID() << "\tsigned..." << std::endl;
		};

		{
			std::cout << "case 0" << std::endl;
			semaphore = std::shared_ptr<YXL::Semaphore>(new YXL::Semaphore(0));

			std::thread t(func);
			t.detach();
			YXL::Sleep(1);

			std::thread t2(func);
			t2.detach();
			YXL::Sleep(1);

			std::cout << YXL::GetCurrentThreadID() << "\tbefore signal..." << std::endl;
			YXL::Sleep(5000);
			semaphore->Signal();
			YXL::Sleep(3000);
			semaphore->Signal();
			std::cout << YXL::GetCurrentThreadID() << "\tafter signal..." << std::endl;
		}
		{
			std::cout << "case 1" << std::endl;
			semaphore = std::shared_ptr<YXL::Semaphore>(new YXL::Semaphore(1));

			std::thread t(func);
			t.detach();
			YXL::Sleep(1);

			std::thread t2(func);
			t2.detach();
			YXL::Sleep(1);

			std::cout << YXL::GetCurrentThreadID() << "\tbefore signal..." << std::endl;
			YXL::Sleep(5000);
			semaphore->Signal();
			YXL::Sleep(3000);
			semaphore->Signal();
			std::cout << YXL::GetCurrentThreadID() << "\tafter signal..." << std::endl;
		}
		{
			std::cout << "case 2" << std::endl;
			semaphore = std::shared_ptr<YXL::Semaphore>(new YXL::Semaphore(0));

			std::thread t(func);
			t.detach();
			YXL::Sleep(1);

			std::cout << YXL::GetCurrentThreadID() << "\tbefore signal..." << std::endl;
			YXL::Sleep(5000);
			semaphore->Signal();
			std::cout << YXL::GetCurrentThreadID() << "\tafter signal..." << std::endl;
		}
	}
}