#pragma once
#include "YXLJsonWarpper.h"
#include "YXLHelper.h"

namespace YXL
{
	namespace Report
	{
		class Report
		{
		public:
			~Report() {}

			virtual void Process() = 0;
			virtual void SaveJson(const std::string& fn) = 0;
			virtual void SavePdf(const std::string& fn)
			{
				auto fn_json = File::GetPathNE(fn) + ".json";
				SaveJson(fn_json);
				Json2PDF(fn_json);
			}

		protected:
			virtual void InitSetting() = 0;

			virtual void Json2PDF(const std::string& fn_json)
			{
				const std::string fn_py = "py/jsonTemp2Pdf.py";
				if (false == File::FileExist(fn_py))
				{
					yxlout << "file not exist: " << File::GetWkDir() + fn_py << std::endl;
					return;
				}
				if (false == File::FileExist(fn_json))
				{
					yxlout << "file not exist: " << fn_json << std::endl;
					return;
				}
				File::RunProgram("python3", fn_py + " " + fn_json, true, false);
			}
		};


		class ReportTable : public Report
		{
		public:
			typedef std::vector<std::string> Row;
			friend struct JSON::ValueParser<ReportTable>;

		public:
			void Process()
			{
				SetRowHeight(_height);
				SetCols(_type, _ratio);
				FillRow();
				InitSetting();
				AddDefaultSetting();
			}
			void SaveJson(const std::string& fn);

			virtual void AddSetting(const int col_idx, const std::string name, const float val)
			{
				if (col_idx < 0 || col_idx >= _ratio.size())
				{
					yxlout << "invalid col_idx..." << std::endl;
					return;
				}
				_setting_float[col_idx][name] = val;
			}
			virtual void AddSetting(const int col_idx, const std::string name, const std::string val)
			{
				if (col_idx < 0 || col_idx >= _ratio.size())
				{
					yxlout << "invalid col_idx..." << std::endl;
					return;
				}
				_setting_str[col_idx][name] = val;
			}

		protected:
			virtual void SetRowHeight(float& h)
			{
				h = 0.1f;
			}
			virtual void SetCols(std::vector<std::string>& type, std::vector<float>& ratio) = 0;
			//return true if row is set
			virtual bool GetNextRow(Row& row) = 0;
			//add some default settings
			virtual void AddDefaultSetting() {}

		private:
			void InitSetting()
			{
				_setting_float.resize(_ratio.size());
				_setting_str.resize(_ratio.size());
			}
			void FillRow()
			{
				Row row;
				while (GetNextRow(row))
					_rows.push_back(row);
			}

		private:
			float _height;
			std::vector<float> _ratio;
			std::vector<std::string> _type;
			std::vector<Row> _rows;
			std::vector<std::map<std::string, float>> _setting_float;
			std::vector<std::map<std::string, std::string>> _setting_str;
		};
	}
}
