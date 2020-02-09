#include <ShellScalingApi.h>
// ATL Includes
#include <atlbase.h>
#include <atlsafe.h>

#include "game_bot.h"
#include "eji_process.h"

#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

void GenData();
int StatusValueRecognitor(cv::Mat im_status_roi, cv::Ptr<cv::ml::KNearest> k_nearest);

/// <summary>
/// ��ɫ״̬��λ����Ϣ
/// </summary>
struct CharacterStatusBarLoc
{
	int x;
	int y;
	int w;
	int h;
};

/// <summary>
/// ��ɫ״̬��Ϣ
/// </summary>
struct CharacterStatusBarInfo
{
	int hp;
	int hp_max;
	int sp;
	int sp_max;
};

/// <summary>
/// ��ʼ����Ϸ��ȡ
/// </summary>
/// <param name="game_pid"></param>
/// <param name="csb_loc"></param>
/// <returns></returns>
EXTERN_DLL_EXPORT void _stdcall GameCaptureInit(int game_pid, CharacterStatusBarLoc& csb_loc)
{
	// ȷ��Windows�����Զ�������Ļ�ߴ�
	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

	std::vector<HWND> hWnds;
	cv::Mat im_src;
	game_bot::get_all_windows_from_pid(game_pid, hWnds);

	for (auto hWnd : hWnds)
	{
		im_src = eji_vision::HWND2Mat(hWnd);
		// �ų����뷨ע�����
		if (im_src.rows > 800)
		{

			// ��λ
			cv::Mat im_template = cv::imread("./template.bmp", CV_8UC1);
			cv::Point match_loc = eji_vision::MatchTemplate(im_src, im_template);

			// ������
			csb_loc.x = match_loc.x;
			csb_loc.y = match_loc.y;
			csb_loc.w = im_template.cols * 10;
			csb_loc.h = im_template.rows;
		}
	}

	GenData();
}

EXTERN_DLL_EXPORT void _stdcall GetStatusBarInfoFromLine(int game_pid, CharacterStatusBarLoc csb_loc, CharacterStatusBarInfo& csb_info)
{
	// ȷ��Windows�����Զ�������Ļ�ߴ�
	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

	std::vector<HWND> hWnds;
	cv::Mat im_src;
	game_bot::get_all_windows_from_pid(game_pid, hWnds);

	for (auto hWnd : hWnds)
	{
		im_src = eji_vision::HWND2Mat(hWnd);
		// �ų����뷨ע�����
		if (im_src.rows > 800)
		{
			cv::Mat im_gray;
			cv::Mat im_thresh;
			cv::cvtColor(im_src, im_gray, cv::COLOR_BGRA2GRAY);
			// ���ж�ֵ������
			cv::threshold(im_gray, im_thresh, 60, 255, cv::THRESH_BINARY);

			// ��ȡ��ɫ״̬��ROI
			cv::Mat im_cbs_roi = im_thresh(cv::Rect(csb_loc.x, csb_loc.y, csb_loc.w, csb_loc.h));

			auto it_hp = cv::LineIterator(im_cbs_roi, cv::Point(18, 38), cv::Point(436, 38));
			auto it_sp = cv::LineIterator(im_cbs_roi, cv::Point(18, 74), cv::Point(436, 74));

			// ʶ��HP
			std::vector<int> hp_collection;
			for (size_t i = 0; i < it_hp.count; i++, ++it_hp)
			{
				hp_collection.push_back((const uchar)*it_hp.ptr);
			}

			// ʶ��SP
			std::vector<int> sp_collection;
			for (size_t i = 0; i < it_sp.count; i++, ++it_sp)
			{
				sp_collection.push_back((const uchar)*it_sp.ptr);
			}

			// ����ٷֱ�
			int hp = 0;
			int sp = 0;
			for (size_t i = 0; i < hp_collection.size(); i++)
			{
				if (hp_collection[i] == 255)
				{
					hp++;
				}

				if (sp_collection[i] == 255)
				{
					sp++;
				}
			}

			csb_info.hp_max = 100;
			csb_info.sp_max = 100;
			csb_info.hp = static_cast<int>((size_t)hp * 100 / hp_collection.size());
			csb_info.sp = static_cast<int>((size_t)sp * 100 / sp_collection.size());

			// ��ʾЧ��
			// cv::rectangle(im_src, match_loc, cv::Point(match_loc.x + im_template.cols * 10, match_loc.y + im_template.rows), cv::Scalar(0, 0, 255), 2);
			// cv::drawContours(im_dst, cnts, 6, cv::Scalar(255, 255, 255), 2);
			// cv::imshow("", im_cbs_roi);
		}
	}
}

EXTERN_DLL_EXPORT void _stdcall GetStatusBarInfoFromOCR(int game_pid, CharacterStatusBarLoc csb_loc, CharacterStatusBarInfo& csb_info)
{
	// ȷ��Windows�����Զ�������Ļ�ߴ�
	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

	std::vector<HWND> hWnds;
	cv::Mat im_src;
	game_bot::get_all_windows_from_pid(game_pid, hWnds);

	for (auto hWnd : hWnds)
	{
		im_src = eji_vision::HWND2Mat(hWnd);
		// �ų����뷨ע�����
		if (im_src.rows > 800)
		{
			cv::Mat im_gray;
			cv::Mat im_thresh;
			cv::cvtColor(im_src, im_gray, cv::COLOR_BGRA2GRAY);
			// ���ж�ֵ������
			cv::threshold(im_gray, im_thresh, 200, 255, cv::THRESH_BINARY);

			// ��ȡ��ɫ״̬��ROI
			cv::Mat im_cbs_roi = im_thresh(cv::Rect(csb_loc.x, csb_loc.y, csb_loc.w, csb_loc.h));

			// �򿪷����ļ�
			cv::FileStorage fs_classifications("DigitRecognition\\Classifications.xml", cv::FileStorage::READ);
			if (fs_classifications.isOpened() == false)
			{
				MessageBox(nullptr, L"Classifications.xml open failed.", L"Error", MB_ICONERROR);
				return;
			}
			cv::Mat classification_ints;
			fs_classifications["classifications"] >> classification_ints;
			fs_classifications.release();

			// ��ѵ��ͼƬ�ļ�
			cv::FileStorage fs_training_images("DigitRecognition\\Images.xml", cv::FileStorage::READ);
			if (fs_training_images.isOpened() == false)
			{
				MessageBox(nullptr, L"Images.xml open failed.", L"Error", MB_ICONERROR);
				return;
			}
			cv::Mat training_images_as_flattened_floats;
			fs_training_images["images"] >> training_images_as_flattened_floats;
			fs_training_images.release();

			// ѵ��
			cv::Ptr<cv::ml::KNearest> k_nearest(cv::ml::KNearest::create());	// ʵ����KNN
			// ����train������������������Mat���ͣ�����Mat��������ʵ�������Ƕ��Ƕ���ͼƬ������
			k_nearest->train(training_images_as_flattened_floats, cv::ml::ROW_SAMPLE, classification_ints);

			// ʶ��HP
			cv::Mat im_dst_hp = im_cbs_roi(cv::Rect(static_cast<int>(im_cbs_roi.cols * 0.2), 0, static_cast<int>(im_cbs_roi.cols * 0.6), static_cast<int>(im_cbs_roi.rows * 0.5)));
			cv::Mat im_dst_curr_hp = im_dst_hp(cv::Rect(0, 0, 131, im_dst_hp.rows));
			cv::Mat im_dst_max_hp = im_dst_hp(cv::Rect(131, 0, (im_dst_hp.cols - 131), im_dst_hp.rows));

			csb_info.hp = StatusValueRecognitor(im_dst_curr_hp, k_nearest);
			csb_info.hp_max = StatusValueRecognitor(im_dst_max_hp, k_nearest);

			// ʶ��SP
			cv::Mat im_dst_sp = im_cbs_roi(cv::Rect(static_cast<int>(im_cbs_roi.cols * 0.2), static_cast<int>(im_cbs_roi.rows * 0.5), static_cast<int>(im_cbs_roi.cols * 0.6), static_cast<int>(im_cbs_roi.rows * 0.5)));
			cv::Mat im_dst_curr_sp = im_dst_sp(cv::Rect(0, 0, 131, im_dst_sp.rows));
			cv::Mat im_dst_max_sp = im_dst_sp(cv::Rect(131, 0, (im_dst_sp.cols - 131), im_dst_sp.rows));

			csb_info.sp = StatusValueRecognitor(im_dst_curr_sp, k_nearest);
			csb_info.sp_max = StatusValueRecognitor(im_dst_max_sp, k_nearest);

			// ��ʾЧ��
			// cv::rectangle(im_src, match_loc, cv::Point(match_loc.x + im_template.cols * 10, match_loc.y + im_template.rows), cv::Scalar(0, 0, 255), 2);
			// cv::drawContours(im_dst, cnts, 6, cv::Scalar(255, 255, 255), 2);
			// cv::imshow("", im_dst_curr_sp);
		}
	}
}

/// <summary>
/// ��ȡ�����б�
/// </summary>
/// <returns></returns>
EXTERN_DLL_EXPORT void _stdcall GetProcessList(SAFEARRAY** pid_arr, SAFEARRAY** proc_name_arr)
{
	eji_process eji_proc;
	eji_proc.get_process_list();

	const int count = static_cast<int>(eji_proc.proc_infoes.size());

	CComSafeArray<int> sa_pid(count);
	for (int i = 0; i < count; i++)
	{
		sa_pid[i] = eji_proc.proc_infoes[i].pid;
	}

	CComSafeArray<BSTR> sa_proc_name(count);
	for (long i = 0; i < count; i++)
	{
		CComBSTR bstr = CComBSTR(static_cast<int>(eji_proc.proc_infoes[i].process_name.size()),
			eji_proc.proc_infoes[i].process_name.data());
		if (bstr == nullptr)
		{
			bstr = L"ϵͳ���н���";
		}
		HRESULT hr = sa_proc_name.SetAt(i, bstr.Detach(), false);
		if (FAILED(hr))
		{
			AtlThrow(hr);
		}
	}

	*pid_arr = sa_pid.Detach();
	*proc_name_arr = sa_proc_name.Detach();
}

/// <summary>
/// ״ֵ̬ʶ����
/// </summary>
/// <param name="im_status_roi"></param>
/// <param name="k_nearest"></param>
/// <returns></returns>
int StatusValueRecognitor(cv::Mat im_status_roi, cv::Ptr<cv::ml::KNearest> k_nearest)
{
	// ȷ����ʼƫ�ƣ���������10��ROI
	im_status_roi = im_status_roi(cv::Rect(3, 0, 120, im_status_roi.rows));
	// ��������ֵ
	std::vector<int> digits_value;
	// hog��������
	cv::HOGDescriptor* hog = new cv::HOGDescriptor(cv::Size(40, 40), cv::Size(16, 16), cv::Size(8, 8), cv::Size(8, 8), 9);

	// ����1��������2����ճ�������
	std::vector<std::vector<cv::Point>> cnts;
	cv::findContours(im_status_roi, cnts, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	cnts.erase(
		std::remove_if(cnts.begin(), cnts.end(),
			[](std::vector<cv::Point>& cnt) { return cnt.size() < 10; }),
		cnts.end());
	for (auto cnt : cnts)
	{
		// ����2����ճ����ʵ�÷���2
		if (cnt.size() > 30)
		{
			digits_value.clear();
			break;
		}

		// ����ʶ��
		cv::Rect bound_rect = cv::boundingRect(cnt);
		cv::Mat im_dst_copy = im_status_roi(bound_rect).clone();
		cv::resize(im_dst_copy, im_dst_copy, cv::Size(40, 40));

		std::vector<float> descriptors;
		hog->compute(im_dst_copy, descriptors);

		cv::Mat roi_flattened_float(1, static_cast<int>(descriptors.size()), CV_32FC1, descriptors.data());
		cv::Mat current_char(0, 0, CV_32F);
		k_nearest->findNearest(roi_flattened_float, 1, current_char);

		int result = (int)current_char.at<float>(0, 0);
		digits_value.push_back(result);
	}

	if (digits_value.size() > 0)
	{
		int status_value = 0;
		size_t size = digits_value.size();
		for (size_t i = 0; i < size; i++)
		{
			status_value += static_cast<int>(digits_value[i] * std::pow(10, i));
		}

		return status_value;
	}

	// ����2���ָ�ÿ������Ϊ12���ROI����
	// ����ÿ������ROI��ROI��������Ϣ
	std::vector<std::vector<cv::Point>> digit_cnts;
	std::vector<cv::Mat> digit_rois;

	for (size_t i = 0; i < 10; i++)
	{
		int offset = static_cast<int>(i * 12);
		cv::Mat im_temp = im_status_roi(cv::Rect(offset, 0, 12, im_status_roi.rows));
		std::vector<std::vector<cv::Point>> cnts;
		cv::findContours(im_temp, cnts, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		/*cv::imshow("", im_temp);
		cv::waitKey(1000);*/
		// �ų��ڿ�
		if (cnts.size() > 0)
		{
			// ȡ�������
			auto max_cnt = *std::max_element(cnts.begin(),
				cnts.end(),
				[](std::vector<cv::Point> const& lhs, std::vector<cv::Point> const& rhs)
				{
					return lhs.size() < rhs.size();
				});

			// ֻ������������
			if (max_cnt.size() > 10)
			{
				digit_cnts.push_back(max_cnt);
				digit_rois.push_back(im_temp);
			}
		}
	}

	for (int i = 0; i < digit_cnts.size(); i++)
	{
		cv::Rect bound_rect = cv::boundingRect(digit_cnts[i]);
		cv::Mat im_dst_copy = digit_rois[i](bound_rect).clone();
		cv::resize(im_dst_copy, im_dst_copy, cv::Size(40, 40));

		std::vector<float> descriptors;
		hog->compute(im_dst_copy, descriptors);

		cv::Mat roi_flattened_float(1, static_cast<int>(descriptors.size()), CV_32FC1, descriptors.data());
		cv::Mat current_char(0, 0, CV_32F);
		k_nearest->findNearest(roi_flattened_float, 1, current_char);

		int result = (int)current_char.at<float>(0, 0);
		digits_value.push_back(result);
	}

	int status_value = 0;
	size_t size = digits_value.size();
	for (size_t i = 0; i < size; i++)
	{
		status_value += static_cast<int>(digits_value[i] * std::pow(10, size - i - 1));
	}

	return status_value;
}

/// <summary>
/// ѵ��ͼƬ��
/// </summary>
void GenData()
{
	cv::Mat classification_ints;					// �������Ǹ���Ȥ���ַ���0~9
	cv::Mat training_images_as_flattened_floats;	// ����ѵ��ͼƬ�е����е����ַ�ROI

	for (int i = 0; i < 10; i++)
	{
		classification_ints.push_back(i);
	}

	int name_arr[10];
	for (int i = 0; i < 10; i++)
	{
		name_arr[i] = i;
	}

	for (int i = 0; i < 10; i++)
	{
		std::stringstream ss;
		ss << name_arr[i];
		std::string name;
		ss >> name;

		cv::Mat im_roi = cv::imread("DigitRecognition\\" + name + ".bmp", CV_8UC1);
		cv::Mat im_roi_copy = im_roi.clone();

		std::vector<std::vector<cv::Point>> cnts;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(im_roi, cnts, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		cv::Rect bound_rect = cv::boundingRect(cnts[0]);
		cv::Mat im_digit = im_roi_copy(bound_rect).clone();
		cv::resize(im_digit, im_digit, cv::Size(40, 40));

		// ����Ŀ��ͼ���HOG�����������ݶ�ֱ��ͼ����������֮ǰ�õĻҶ�����
		cv::HOGDescriptor* hog = new cv::HOGDescriptor(cv::Size(40, 40), cv::Size(16, 16), cv::Size(8, 8), cv::Size(8, 8), 9);
		std::vector<float> descriptors;
		hog->compute(im_digit, descriptors);
		cv::Mat im_dst(1, static_cast<int>(descriptors.size()), CV_32FC1, descriptors.data());

		training_images_as_flattened_floats.push_back(im_dst);
	}

	// ��������ļ�
	cv::FileStorage fs_classifications("DigitRecognition\\Classifications.xml", cv::FileStorage::WRITE);

	if (fs_classifications.isOpened() == false)
	{
		MessageBox(nullptr, L"Classifications.xml open failed.", L"Error", MB_ICONERROR);
		return;
	}

	fs_classifications << "classifications" << classification_ints;
	fs_classifications.release();

	// ����ѵ��ͼƬ�ļ�
	cv::FileStorage fs_training_images("DigitRecognition\\Images.xml", cv::FileStorage::WRITE);
	if (fs_training_images.isOpened() == false)
	{
		MessageBox(nullptr, L"Images.xml open failed.", L"Error", MB_ICONERROR);
		return;
	}
	fs_training_images << "images" << training_images_as_flattened_floats;
	fs_training_images.release();
}