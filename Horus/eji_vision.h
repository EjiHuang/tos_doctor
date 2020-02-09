#pragma once

#include <opencv.hpp>
#include <iostream>
#include <Windows.h>

class eji_vision
{
public:
	/// <summary>
	/// HWNDתMat
	/// </summary>
	/// <param name="hwnd"></param>
	/// <returns></returns>
	static cv::Mat HWND2Mat(HWND hwnd)
	{
		HDC hdc_window, hdc_window_compatible;
		HBITMAP hb_window;
		cv::Mat im_src;
		BITMAPINFOHEADER bmp_info;

		hdc_window = GetDC(hwnd);								// ��ȡ���ھ�����豸������
		hdc_window_compatible = CreateCompatibleDC(hdc_window);		// ��ȡ�豸�����ĵ��ڴ���
		SetStretchBltMode(hdc_window_compatible, COLORONCOLOR);		// ����λͼ������ģʽ���Ա㵱ͼ�����Ϊ��С�ߴ�ʱ�������������ڲ�������Ϣ������±�ɾ��

		// ���ڵĳߴ�
		RECT window_size;
		GetClientRect(hwnd, &window_size);

		auto width = window_size.right;
		auto height = window_size.bottom;

		// ������ɫ����
		im_src.create(height, width, CV_8UC4);

		// ����λͼ
		hb_window = CreateCompatibleBitmap(hdc_window, width, height);
		bmp_info.biSize = sizeof(BITMAPINFOHEADER);
		bmp_info.biWidth = width;
		bmp_info.biHeight = -height;  // ����ʹ�����µߵ�����
		bmp_info.biPlanes = 1;
		bmp_info.biBitCount = 32;
		bmp_info.biCompression = BI_RGB;
		bmp_info.biSizeImage = 0;
		bmp_info.biXPelsPerMeter = 0;
		bmp_info.biYPelsPerMeter = 0;
		bmp_info.biClrUsed = 0;
		bmp_info.biClrImportant = 0;

		// ʹ����ǰ�������豸����������λͼ
		SelectObject(hdc_window_compatible, hb_window);
		// �Ӵ����豸�����ĸ��Ƶ�λͼ�豸������
		StretchBlt(hdc_window_compatible, 0, 0, width, height, hdc_window, 0, 0, width, height, SRCCOPY);
		GetDIBits(hdc_window_compatible, hb_window, 0, height, im_src.data, (BITMAPINFO*)&bmp_info, DIB_RGB_COLORS);

		// �ͷ���Դ
		DeleteObject(hb_window);
		DeleteDC(hdc_window_compatible);
		ReleaseDC(hwnd, hdc_window);

		return im_src;
	}

	/// <summary>
	/// ����ģ��ƥ����
	/// </summary>
	/// <param name="im_src"></param>
	/// <param name="template_path"></param>
	/// <returns></returns>
	static cv::Point MatchTemplate(cv::Mat im_src, cv::Mat im_template)
	{
		cv::Mat im_gray;
		cv::Mat im_thresh;
		cv::cvtColor(im_src, im_gray, cv::COLOR_BGRA2GRAY);
		// ���ж�ֵ������
		cv::threshold(im_gray, im_thresh, 200, 255, cv::THRESH_BINARY);

		// ����ģ��ƥ�䣬��λ��ɫ״̬��
		cv::Mat im_match_result;
		cv::matchTemplate(im_thresh, im_template, im_match_result, cv::TM_CCORR_NORMED);
		// ��һ�����
		cv::normalize(im_match_result, im_match_result, 0, 1, cv::NORM_MINMAX);
		// ��λ���ƥ��λ��
		double min_value, max_value;
		cv::Point min_loc, max_loc;
		cv::Point match_loc;
		cv::minMaxLoc(im_match_result, &min_value, &max_value, &min_loc, &max_loc, cv::Mat());
		// ���ڷ���SQDIFF��SQDIFF_NORMED���ַ���������ԽС��ֵ�����Ÿ��ߵ�ƥ������������ķ���������ֵԽ��ƥ��Ч��Խ��
		match_loc = max_loc;

		return match_loc;
	}
};
