
// LogisticModelDlg.cpp: файл реализации
//

#include "pch.h"
#include "framework.h"
#include "LogisticModel.h"
#include "LogisticModelDlg.h"
#include "afxdialogex.h"

#include <math.h>
#include <iostream>
#include <fstream>
#include <random>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DOTS(x,y) (xp*((x)-xmin)),(yp*((y)-ymax))

using namespace std;

// Диалоговое окно CLogisticModelDlg

struct PRNG
{
	std::mt19937 engine;
};

void initGenerator(PRNG& generator)
{
	// Создаём псевдо-устройство для получения случайного зерна.
	std::random_device device;
	// Получаем случайное зерно последовательности
	generator.engine.seed(device());
}

double getRandomdouble(PRNG& generator, double minValue, double maxValue)
{
	// Проверяем корректность аргументов
	assert(minValue < maxValue);

	// Создаём распределение
	std::uniform_real_distribution<double> distribution(minValue, maxValue);

	// Вычисляем псевдослучайное число: вызовем распределение как функцию,
	//  передав генератор произвольных целых чисел как аргумент.
	return distribution(generator.engine);
}

void CLogisticModelDlg::Mashtab(double arr[], int dim, double* mmin, double* mmax)		//определяем функцию масштабирования
{
	*mmin = *mmax = arr[0];

	for (int i = 0; i < dim; i++)
	{
		if (*mmin > arr[i]) *mmin = arr[i];
		if (*mmax < arr[i]) *mmax = arr[i];
	}
}

CLogisticModelDlg::CLogisticModelDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LOGISTICMODEL_DIALOG, pParent)
	, x0(0.442)
	, R_koef(3.0)
	, Length(200)
	, num_of_counts(100)
	, precision(1.E-3)
	, b1_dot(0)
	, b2_up_dot(0)
	, b2_down_dot(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLogisticModelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_X0, x0);
	DDX_Text(pDX, IDC_EDIT_R, R_koef);
	DDX_Text(pDX, IDC_EDIT_N, Length);
	DDX_Control(pDX, IDC_CHECK_GEN_X0, GenerationX0);
	DDX_Control(pDX, IDC_EDIT_X0, Edit_x0);
	DDX_Control(pDX, IDC_CHECK_GEN_R, GenerationR);
	DDX_Control(pDX, IDC_EDIT_R, Edit_R);
	//  DDX_Control(pDX, IDC_BUTTON_EXIT, GraphSignature);
	DDX_Control(pDX, IDC_RADIO_LOGISTIC, m_radio_logistic);
	DDX_Control(pDX, IDC_RADIO_BIFUR, m_radio_bifur);
	DDX_Control(pDX, IDC_SIGNATURE, GraphSignature);
	DDX_Text(pDX, IDC_EDIT_NUMBER_OF_COUNTS, num_of_counts);
	DDX_Text(pDX, IDC_EDIT_PRECISION, precision);
	DDX_Control(pDX, IDC_EDIT_N, Edit_Length);
	DDX_Control(pDX, IDC_EDIT_NUMBER_OF_COUNTS, Edit_M);
	DDX_Control(pDX, IDC_EDIT_PRECISION, Edit_Prec);
	DDX_Control(pDX, IDC_RADIO_DOTS_DRAW, dots_draw);
	DDX_Control(pDX, IDC_RADIO_LINES_DRAW, lines_draw);
	DDX_Text(pDX, IDC_STATIC_BIF_DOT_B1, b1_dot);
	DDX_Text(pDX, IDC_STATIC_BIF_DOT_B2_UP, b2_up_dot);
	DDX_Text(pDX, IDC_STATIC_BIF_DOT_B2_DOWN, b2_down_dot);
}

BEGIN_MESSAGE_MAP(CLogisticModelDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CLogisticModelDlg::OnBnClickedButtonExit)
	ON_BN_CLICKED(IDC_BUTTON_START, &CLogisticModelDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_CHECK_GEN_X0, &CLogisticModelDlg::OnBnClickedCheckGenX0)
	ON_BN_CLICKED(IDC_CHECK_GEN_R, &CLogisticModelDlg::OnBnClickedCheckGenR)
	ON_BN_CLICKED(IDC_RADIO_BIFUR, &CLogisticModelDlg::OnBnClickedRadioBifur)
	ON_BN_CLICKED(IDC_RADIO_LOGISTIC, &CLogisticModelDlg::OnBnClickedRadioLogistic)
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// Обработчики сообщений CLogisticModelDlg

BOOL CLogisticModelDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Задает значок для этого диалогового окна.  Среда делает это автоматически,
	//  если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок

	// TODO: добавьте дополнительную инициализацию

	PicWnd = GetDlgItem(IDC_GRAPH);			//связываем с ID окон
	PicDc = PicWnd->GetDC();
	PicWnd->GetClientRect(&Pic);

	// перья
	setka_pen.CreatePen(		//для сетки
		PS_DOT,					//пунктирная
		1,						//толщина 1 пиксель
		RGB(0, 0, 0));			//цвет  черный

	osi_pen.CreatePen(			//координатные оси
		PS_SOLID,				//сплошная линия
		3,						//толщина 2 пикселя
		RGB(0, 0, 0));			//цвет черный

	signal_pen.CreatePen(			//график
		PS_SOLID,				//сплошная линия
		-1,						//толщина -1 пикселя
		RGB(0, 0, 255));			//цвет синий

	bifurcation_dots_pen.CreatePen(			//график
		PS_SOLID,				//сплошная линия
		3,						//толщина -1 пикселя
		RGB(255, 0, 0));			//цвет синий

	bifurcation_lines_pen.CreatePen(			//график
		PS_SOLID,				//сплошная линия
		-1,						//толщина -1 пикселя
		RGB(255, 0, 0));			//цвет синий

	UpdateData(false);
	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

// При добавлении кнопки свертывания в диалоговое окно нужно воспользоваться приведенным ниже кодом,
//  чтобы нарисовать значок.  Для приложений MFC, использующих модель документов или представлений,
//  это автоматически выполняется рабочей областью.

void CLogisticModelDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Нарисуйте значок
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// Система вызывает эту функцию для получения отображения курсора при перемещении
//  свернутого окна.
HCURSOR CLogisticModelDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CLogisticModelDlg::DrawSignal(double* Mass, CDC* WinDc, CRect WinPic, CPen* graphpen, double AbsMax)
{
	// поиск максимального и минимального значения
	Mashtab(Mass, AbsMax, &ymin, &ymax);


	ymax = 1.5 * ymax;
	ymin = -ymax * 0.1;
	xmax = Length;
	xmin = -xmax * 0.1;

	// создание контекста устройства
	CBitmap bmp;
	CDC* MemDc;
	MemDc = new CDC;
	MemDc->CreateCompatibleDC(WinDc);

	double window_signal_width = WinPic.Width() * scale;
	double window_signal_height = WinPic.Height() * scale;
	xp = (window_signal_width / (xmax - xmin));			//Коэффициенты пересчёта координат по Х
	yp = -(window_signal_height / (ymax - ymin));			//Коэффициенты пересчёта координат по У

	bmp.CreateCompatibleBitmap(WinDc, window_signal_width, window_signal_height);
	CBitmap* pBmp = (CBitmap*)MemDc->SelectObject(&bmp);
	// заливка фона графика белым цветом
	MemDc->FillSolidRect(WinPic, RGB(255, 255, 255));
	// отрисовка сетки координат
	MemDc->SelectObject(&osi_pen);

	//создаём Ось Y
	MemDc->MoveTo(DOTS(defaultX0, ymax));
	MemDc->LineTo(DOTS(defaultX0, ymin));
	//создаём Ось Х
	MemDc->MoveTo(DOTS(xmin, -defaultY0));
	MemDc->LineTo(DOTS(xmax, -defaultY0));

	MemDc->SelectObject(&setka_pen);
	//отрисовка сетки по y
	for (double x = 0; x <= xmax; x += xmax / scale / 8)
	{
		if (x != 0) {
			MemDc->MoveTo(DOTS(x, ymax));
			MemDc->LineTo(DOTS(x, ymin));
		}
	}
	//отрисовка сетки по x
	for (double y = -ymax; y < ymax; y += ymax / scale / 4)
	{
		if (y != 0) {
			MemDc->MoveTo(DOTS(xmin, y));
			MemDc->LineTo(DOTS(xmax, y));
		}
	}

	// установка прозрачного фона текста
	MemDc->SetBkMode(TRANSPARENT);
	// установка шрифта
	CFont font;
	font.CreateFontW(14.5, 0, 0, 0, FW_HEAVY, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS || CLIP_LH_ANGLES, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Century Gothic"));
	MemDc->SelectObject(&font);

	//подпись осей
	MemDc->TextOutW(DOTS(0.02 * xmax, 0.98 * ymax), _T("Xn")); //Y
	MemDc->TextOutW(DOTS(xmax - xmax / 15, 0.2 * ymax), _T("N")); //X

	//по Y с шагом 5
	for (double i = -ymax; i <= ymax; i += ymax / scale / 4)
	{
		CString str;
		if (i != 0)
		{
			str.Format(_T("%.3f"), i / scale + defaultY0 / scale);
			MemDc->TextOutW(DOTS(defaultX0 - xmax / 12, i + 0.03 * ymax), str);
		}
	}
	//по X с шагом 0.5
	for (double j = 0; j <= xmax; j += xmax / scale / 8)
	{
		CString str;
		if (j != 0) {
			str.Format(_T("%.1f"), j / scale - defaultX0 / scale);
			MemDc->TextOutW(DOTS(j - xmax / 100, -defaultY0), str);
		}
	}

	// отрисовка
	MemDc->SelectObject(graphpen);
	MemDc->MoveTo(DOTS(defaultX0, Mass[0] * scale - defaultY0));
	for (int i = 1; i < AbsMax; i++)
	{
		MemDc->LineTo(DOTS(i + defaultX0, Mass[i] * scale - defaultY0));
	}

	// вывод на экран
	WinDc->BitBlt(0, 0, window_signal_width, window_signal_height, MemDc, 0, 0, SRCCOPY);
	delete MemDc;
}

void CLogisticModelDlg::DrawDotsBifurcation(Points* Mass, CDC* WinDc, CRect WinPic, CPen* graphpen, int AbsMax)
{
	// поиск максимального и минимального значения
	/*Mashtab(&Mass->y_x1000, AbsMax, &ymin, &ymax);*/

	ymax = 1;
	ymin = -ymax * 0.1;
	xmax = 4;
	xmin = -xmax * 0.1;

	// создание контекста устройства
	CBitmap bmp;
	CDC* MemDc;
	MemDc = new CDC;
	MemDc->CreateCompatibleDC(WinDc);

	double window_signal_width = WinPic.Width() * scale;
	double window_signal_height = WinPic.Height() * scale;
	xp = (window_signal_width / (xmax - xmin));			//Коэффициенты пересчёта координат по Х
	yp = -(window_signal_height / (ymax - ymin));			//Коэффициенты пересчёта координат по У

	bmp.CreateCompatibleBitmap(WinDc, window_signal_width, window_signal_height);
	CBitmap* pBmp = (CBitmap*)MemDc->SelectObject(&bmp);
	// заливка фона графика белым цветом
	MemDc->FillSolidRect(WinPic, RGB(255, 255, 255));
	// отрисовка сетки координат
	MemDc->SelectObject(&osi_pen);

	//создаём Ось Y
	MemDc->MoveTo(DOTS(defaultX0, ymax));
	MemDc->LineTo(DOTS(defaultX0, ymin));
	//создаём Ось Х
	MemDc->MoveTo(DOTS(xmin, -defaultY0));
	MemDc->LineTo(DOTS(xmax, -defaultY0));

	MemDc->SelectObject(&setka_pen);
	//отрисовка сетки по y
	for (double x = 0; x <= xmax; x += xmax / scale / 8)
	{
		if (x != 0) {
			MemDc->MoveTo(DOTS(x, ymax));
			MemDc->LineTo(DOTS(x, ymin));
		}
	}
	//отрисовка сетки по x
	for (double y = -ymax; y < ymax; y += ymax / scale / 4)
	{
		if (y != 0) {
			MemDc->MoveTo(DOTS(xmin, y));
			MemDc->LineTo(DOTS(xmax, y));
		}
	}

	// установка прозрачного фона текста
	MemDc->SetBkMode(TRANSPARENT);
	// установка шрифта
	CFont font;
	font.CreateFontW(14.5, 0, 0, 0, FW_HEAVY, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS || CLIP_LH_ANGLES, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Century Gothic"));
	MemDc->SelectObject(&font);

	//подпись осей
	MemDc->TextOutW(DOTS(0.02 * xmax, 0.98 * ymax), _T("x1000")); //Y
	MemDc->TextOutW(DOTS(xmax - xmax / 15, 0.2 * ymax), _T("R")); //X

	//по Y с шагом 5
	for (double i = -ymax; i <= ymax; i += ymax / scale / 4)
	{
		CString str;
		if (i != 0)
		{
			str.Format(_T("%.3f"), i / scale + defaultY0 / scale);
			MemDc->TextOutW(DOTS(xmin + xmax / 50, i + 0.03 * ymax), str);
		}
	}
	//по X с шагом 0.5
	for (double j = 0; j <= xmax; j += xmax / scale / 8)
	{
		CString str;
		if (j != 0) {
			str.Format(_T("%.1f"), j / scale - defaultX0 / scale);
			MemDc->TextOutW(DOTS(j - xmax / 100, -defaultY0), str);
		}
	}

	// отрисовка
	MemDc->SelectObject(graphpen);
	for (int i = 0; i < AbsMax; i++)
	{
		MemDc->Ellipse(DOTS(Mass[i].x_R * scale + defaultX0 - xmax * 0.001, Mass[i].y_x1000 * scale - defaultY0 - ymax * 0.001), DOTS(Mass[i].x_R * scale + defaultX0 + xmax * 0.001, Mass[i].y_x1000 * scale - defaultY0 + ymax * 0.001));
	}

	// вывод на экран
	WinDc->BitBlt(0, 0, window_signal_width, window_signal_height, MemDc, 0, 0, SRCCOPY);
	delete MemDc;
}

void CLogisticModelDlg::DrawLinesBifurcation(PointsToLine* Mass, CDC* WinDc, CRect WinPic, CPen* graphpen, int AbsMax)
{
	// поиск максимального и минимального значения
	/*Mashtab(&Mass->y_x1000, AbsMax, &ymin, &ymax);*/

	ymax = 1;
	ymin = -ymax * 0.1;
	xmax = 4;
	xmin = -xmax * 0.1;

	// создание контекста устройства
	CBitmap bmp;
	CDC* MemDc;
	MemDc = new CDC;
	MemDc->CreateCompatibleDC(WinDc);

	double window_signal_width = WinPic.Width() * scale;
	double window_signal_height = WinPic.Height() * scale;
	xp = (window_signal_width / (xmax - xmin));			//Коэффициенты пересчёта координат по Х
	yp = -(window_signal_height / (ymax - ymin));			//Коэффициенты пересчёта координат по У

	bmp.CreateCompatibleBitmap(WinDc, WinPic.Width(), WinPic.Height());
	CBitmap* pBmp = (CBitmap*)MemDc->SelectObject(&bmp);
	// заливка фона графика белым цветом
	MemDc->FillSolidRect(WinPic, RGB(255, 255, 255));
	// отрисовка сетки координат
	MemDc->SelectObject(&osi_pen);

	//создаём Ось Y
	MemDc->MoveTo(DOTS(defaultX0, ymax));
	MemDc->LineTo(DOTS(defaultX0, ymin));
	//создаём Ось Х
	MemDc->MoveTo(DOTS(xmin, -defaultY0));
	MemDc->LineTo(DOTS(xmax, -defaultY0));

	MemDc->SelectObject(&setka_pen);
	//отрисовка сетки по y
	for (double x = 0; x <= xmax; x += xmax / scale / 8)
	{
		if (x != 0) {
			MemDc->MoveTo(DOTS(x, ymax));
			MemDc->LineTo(DOTS(x, ymin));
		}
	}
	//отрисовка сетки по x
	for (double y = -ymax; y < ymax; y += ymax / scale / 4)
	{
		if (y != 0) {
			MemDc->MoveTo(DOTS(xmin, y));
			MemDc->LineTo(DOTS(xmax, y));
		}
	}

	// установка прозрачного фона текста
	MemDc->SetBkMode(TRANSPARENT);
	// установка шрифта
	CFont font;
	font.CreateFontW(14.5, 0, 0, 0, FW_HEAVY, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS || CLIP_LH_ANGLES, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Century Gothic"));
	MemDc->SelectObject(&font);

	//подпись осей
	MemDc->TextOutW(DOTS(0.02 * xmax, 0.98 * ymax), _T("x1000")); //Y
	MemDc->TextOutW(DOTS(xmax - xmax / 15, 0.2 * ymax), _T("R")); //X

	//по Y с шагом 5
	for (double i = -ymax; i <= ymax; i += ymax / scale / 4)
	{
		CString str;
		if (i != 0)
		{
			str.Format(_T("%.3f"), i / scale + defaultY0 / scale);
			MemDc->TextOutW(DOTS(xmin + xmax / 50, i + 0.03 * ymax), str);
		}
	}
	//по X с шагом 0.5
	for (double j = 0; j <= xmax; j += xmax / scale / 8)
	{
		CString str;
		if (j != 0) {
			str.Format(_T("%.1f"), j / scale - defaultX0 / scale);
			MemDc->TextOutW(DOTS(j - xmax / 100, -defaultY0), str);
		}
	}

	// отрисовка
	MemDc->SelectObject(graphpen);
	for (int i = 0; i < AbsMax; i++)
	{
		MemDc->MoveTo(DOTS(Mass[i].x_R * scale + defaultX0, Mass[i].y_x1000 * scale - defaultY0));
		MemDc->LineTo(DOTS(Mass[Mass[i].IndexPreviousPoint].x_R * scale + defaultX0, Mass[Mass[i].IndexPreviousPoint].y_x1000 * scale - defaultY0));
	}

	// вывод на экран
	WinDc->BitBlt(0, 0, window_signal_width, window_signal_height, MemDc, 0, 0, SRCCOPY);
	delete MemDc;
}

void CLogisticModelDlg::Signal(int t)
{
	delete[] sign;
	sign = new double[t];
	for (int i = 0; i < t; i++)
	{
		sign[i] = 0;
	}

	sign[0] = x0;

	for (int i = 1; i < t; i++)
	{
		sign[i] = R_koef * sign[i - 1] * (1 - sign[i - 1]);
	}
}

void CLogisticModelDlg::UniqArrayWithPrecision(double* arr, int size, int& NewSize)
{
	for (int counter1 = 0; counter1 < size; counter1++)
	{
		for (int counter2 = counter1 + 1; counter2 < size; counter2++)
		{
			if (fabs(arr[counter1] - arr[counter2]) <= precision) // если найден одинаковый элемент
			{
				for (int counter_shift = counter2; counter_shift < size - 1; counter_shift++)
				{
					// выполнить сдвиг всех остальных элементов массива на -1, начиная со следующего элемента, после найденного дубля
					arr[counter_shift] = arr[counter_shift + 1];
				}
				size -= 1; // уменьшить размер массива на 1

				if (fabs(arr[counter1] - arr[counter2]) <= precision) // если следующий элемент - дубль
				{
					counter2--; // выполнить переход на предыдущий элемент     
				}
			}
		}
	}
	NewSize = size;
}

void CLogisticModelDlg::OnBnClickedButtonExit()
{
	// TODO: добавьте свой код обработчика уведомлений
	CDialogEx::OnCancel();
}

void CLogisticModelDlg::OnBnClickedButtonStart()
{
	// TODO: добавьте свой код обработчика уведомлений
	UpdateData(TRUE);

	if (m_radio_bifur.GetCheck() == BST_UNCHECKED && m_radio_logistic.GetCheck() == BST_UNCHECKED)
	{
		MessageBox(L"Необходимо выбрать тип задачи для дальнейших расчетов!", L"Warning", MB_ICONINFORMATION | MB_OK);
	}

	if (m_radio_logistic.GetCheck() == BST_CHECKED)
	{
		if (GenerationX0.GetCheck() == BST_CHECKED)
		{
			PRNG generator;
			initGenerator(generator);
			double from = 0;
			double to = 1;

			x0 = getRandomdouble(generator, from, to);
		}

		if (GenerationR.GetCheck() == BST_CHECKED)
		{
			PRNG generator;
			initGenerator(generator);
			double from = 0.000001;
			double to = 4.000001;

			R_koef = getRandomdouble(generator, from, to);
		}

		Signal(Length);
		DrawSignal(sign, PicDc, Pic, &signal_pen, Length);

		GraphSignature.SetWindowText(_T("Рис. Логистическая модель (N членов ряда Xn)."));

		/*int NewSize = 0;
		double x1000[5] = { 0.1111, 3.5181, 84.12, 0.1112, 84.12 };

		ofstream out("unicarraytest.txt");
		out << "Исходный массив: " << endl;
		for (int i = 0; i < 5; i++)
		{
			out << x1000[i] << endl;
		}
		out << endl << "Количество элементов: " << 5;
		out << endl << endl;
		out << "Уникальный массив: " << endl;
		UniqArrayWithPrecision(x1000, 5, NewSize);
		for (int i = 0; i < NewSize; i++)
		{
			out << x1000[i] << endl;
		}
		out << endl << "Количество элементов: " << NewSize;
		delete[] unicArr;
		out.close();*/

		UpdateData(false);
		//delete[] sign;
	}

	if (m_radio_bifur.GetCheck() == BST_CHECKED)
	{
		if (dots_draw.GetCheck() == BST_UNCHECKED && lines_draw.GetCheck() == BST_UNCHECKED)
		{
			MessageBox(L"Для выполнения расчётов выберете режим отрисовки графика!", L"Warning", MB_ICONINFORMATION | MB_OK);
		}
		double R_Length = (R_to - R_from) / R_Step + 1;

		int k = 1001;

		PRNG generator;
		initGenerator(generator);
		double from = 0;
		double to = 1;

		vector<double> VectorX1000;
		vector<double> VectorR;
		VectorX1000.clear();
		VectorR.clear();

		if (dots_draw.GetCheck() == BST_CHECKED)
		{
			delete[] BifurcationDots;
			delete[] BifurcationLines;

			double* x1000 = new double[num_of_counts] {0};

			for (double r = R_from; r <= R_to; r += R_Step)
			{
				R_koef = r;
				double buff = 0;
				int NewSize = 0;

				for (int i = 0; i < num_of_counts; i++)
				{
					x1000[i] = 0;
				}

				for (int num = 0; num < num_of_counts; num++)
				{
					x0 = getRandomdouble(generator, from, to);
					Signal(k);
					double difference = 0;

					x1000[num] = sign[k - 1];
				}
				UniqArrayWithPrecision(x1000, num_of_counts, NewSize);

				for (int i = 0; i < NewSize; i++)
				{
					VectorX1000.push_back(x1000[i]);
					VectorR.push_back(r);
				}
			}

			bifurSize = 0;
			//ofstream out("bifurcation_dots_struct.txt");
			///*out << VectorR.size() << endl << VectorX1000.size();*/
			if (VectorR.size() == VectorX1000.size())
			{
				bifurSize = VectorR.size();
				BifurcationDots = new Points[bifurSize];
				BifurcationLines = new PointsToLine[bifurSize];
				for (int i = 0; i < bifurSize; i++)
				{
					BifurcationDots[i] = { 0, 0 };
					BifurcationDots[i] = { VectorR[i], VectorX1000[i] };

					/*out << endl << "Значение R: " << BifurcationDots[i].x_R << "\tЗначение X1000: " << BifurcationDots[i].y_x1000 << endl << endl << endl;*/
				}
				/*out.close();*/

				for (int i = 0; i < bifurSize; i++)
				{
					if (BifurcationDots[i].x_R != BifurcationDots[i + 1].x_R && BifurcationDots[i + 1].x_R == BifurcationDots[i + 2].x_R && BifurcationDots[i].x_R <= 0.75 * R_to)
					{
						b1_dot = BifurcationDots[i].x_R;
						break;
					}
				}

				for (int i = 0; i < bifurSize; i++)
				{
					if (BifurcationDots[i].x_R == BifurcationDots[i + 1].x_R && BifurcationDots[i + 2].x_R == BifurcationDots[i + 3].x_R
						&& BifurcationDots[i + 2].x_R == BifurcationDots[i + 4].x_R && BifurcationDots[i + 2].x_R != BifurcationDots[i].x_R
						&& BifurcationDots[i + 4].x_R == BifurcationDots[i + 5].x_R && BifurcationDots[i + 5].x_R != BifurcationDots[i + 6].x_R)
					{
						b2_up_dot = BifurcationDots[i].x_R;
						b2_down_dot = BifurcationDots[i].x_R;
						break;
					}
				}

				DrawDotsBifurcation(BifurcationDots, PicDc, Pic, &bifurcation_dots_pen, bifurSize);

				GraphSignature.SetWindowText(_T("Рис. Бифуркационная диаграмма логистического отображения (точ)."));
				delete[] x1000;

				UpdateData(false);
			}
		}
		if (lines_draw.GetCheck() == BST_CHECKED)
		{
			delete[] BifurcationDots;
			delete[] BifurcationLines;

			double* x1000 = new double[num_of_counts] {0};

			for (double r = R_from; r <= R_to; r += R_Step)
			{
				R_koef = r;
				double buff = 0;
				int NewSize = 0;

				for (int i = 0; i < num_of_counts; i++)
				{
					x1000[i] = 0;
				}

				for (int num = 0; num < num_of_counts; num++)
				{
					x0 = getRandomdouble(generator, from, to);
					Signal(k);
					double difference = 0;

					x1000[num] = sign[k - 1];
				}
				UniqArrayWithPrecision(x1000, num_of_counts, NewSize);

				for (int i = 0; i < NewSize; i++)
				{
					VectorX1000.push_back(x1000[i]);
					VectorR.push_back((double)r);
				}
			}

			bifurSize = 0;
			/*ofstream out("bifurcation_dots_struct.txt");*/
			/*out << VectorR.size() << endl << VectorX1000.size();*/
			if (VectorR.size() == VectorX1000.size())
			{
				bifurSize = VectorR.size();
				BifurcationDots = new Points[bifurSize];
				BifurcationLines = new PointsToLine[bifurSize];
				for (int i = 0; i < bifurSize; i++)
				{
					BifurcationDots[i] = { 0, 0 };
					if (i != 0) BifurcationLines[i] = { 0, 0, i - 1 };
					else BifurcationLines[i] = { 0, 0, 0 };

					BifurcationDots[i] = { (double)VectorR[i], (double)VectorX1000[i] };

					/*out << endl << "Значение R: " << BifurcationDots[i].x_R << "\tЗначение X1000: " << BifurcationDots[i].y_x1000 << endl << endl << endl;*/
				}
				/*	out.close();*/

				vector<int> IndexVec;
				int PreviousIndex = 0;

				double x_i = 0;
				double y_i = 0;

				for (int i = 0; i < bifurSize; i++)
				{
					PreviousIndex = 0;

					x_i = (double)VectorR[i];
					y_i = (double)VectorX1000[i];

					if (x_i != 0.0)
					{
						double x_prev = x_i - R_Step;
						for (int j = 0; j < i; j++)
						{
							if (BifurcationDots[j].x_R - x_prev < precision)
							{
								IndexVec.push_back(j);
							}
						}
						if (IndexVec.empty() == false)
						{
							if (IndexVec.size() == 1)
							{
								BifurcationLines[i] = { x_i, y_i, IndexVec[0] };
							}
							else
							{
								double min = (x_i - BifurcationDots[IndexVec[0]].x_R) * (x_i - BifurcationDots[IndexVec[0]].x_R) + (y_i - BifurcationDots[IndexVec[0]].y_x1000) * (y_i - BifurcationDots[IndexVec[0]].y_x1000);
								for (int j = 0; j < IndexVec.size(); j++)
								{
									if (min > (x_i - BifurcationDots[IndexVec[j]].x_R) * (x_i - BifurcationDots[IndexVec[j]].x_R) + (y_i - BifurcationDots[IndexVec[j]].y_x1000) * (y_i - BifurcationDots[IndexVec[j]].y_x1000))
									{
										min = (x_i - BifurcationDots[IndexVec[j]].x_R) * (x_i - BifurcationDots[IndexVec[j]].x_R) + (y_i - BifurcationDots[IndexVec[j]].y_x1000) * (y_i - BifurcationDots[IndexVec[j]].y_x1000);
										PreviousIndex = IndexVec[j];
									}
								}
								BifurcationLines[i] = { x_i, y_i, PreviousIndex };
							}
						}
						else
						{
							PreviousIndex = i - 1;
							BifurcationLines[i] = { x_i, y_i, PreviousIndex };
						}
					}
					else
					{
						if (i == 0)
						{
							PreviousIndex = 0;
							BifurcationLines[i] = { x_i, y_i, PreviousIndex };
						}
						else
						{
							PreviousIndex = i - 1;
							BifurcationLines[i] = { x_i, y_i, PreviousIndex };
						}
					}
					IndexVec.clear();
				}

				/*ofstream outt("bifLines.txt");
				for (int i = 0; i < bifurSize; i++)
				{
					outt << "Index: " << i << "\tR: " << BifurcationLines[i].x_R << "\t\tPrev.Index: " << BifurcationLines[i].IndexPreviousPoint << "\t\tX1000: " << BifurcationLines[i].y_x1000 << endl << endl;
				}*/

				for (int i = 0; i < bifurSize; i++)
				{
					if (BifurcationLines[i].x_R != BifurcationLines[i + 1].x_R && BifurcationLines[i + 1].x_R == BifurcationLines[i + 2].x_R && BifurcationLines[i].x_R <= 0.75 * R_to)
					{
						b1_dot = BifurcationLines[i].x_R;
						break;
					}
				}

				for (int i = 0; i < bifurSize; i++)
				{
					if (BifurcationLines[i].x_R == BifurcationLines[i + 1].x_R && BifurcationLines[i + 2].x_R == BifurcationLines[i + 3].x_R
						&& BifurcationLines[i + 2].x_R == BifurcationLines[i + 4].x_R && BifurcationLines[i + 2].x_R != BifurcationLines[i].x_R
						&& BifurcationLines[i + 4].x_R == BifurcationLines[i + 5].x_R && BifurcationLines[i + 5].x_R != BifurcationLines[i + 6].x_R)
					{
						b2_up_dot = BifurcationLines[i].x_R;
						b2_down_dot = BifurcationLines[i].x_R;
						break;
					}
				}

				DrawLinesBifurcation(BifurcationLines, PicDc, Pic, &bifurcation_lines_pen, bifurSize);
				GraphSignature.SetWindowText(_T("Рис. Бифуркационная диаграмма логистического отображения (непр)."));

				delete[] x1000;
				UpdateData(false);
				/*delete[] BifurcationDots;
				delete[] BifurcationLines;*/
			}
		}
	}
}


void CLogisticModelDlg::OnOK()
{
	// TODO: добавьте специализированный код или вызов базового класса

	//CDialogEx::OnOK();
	OnBnClickedButtonStart();
}


void CLogisticModelDlg::OnBnClickedCheckGenX0()
{
	// TODO: добавьте свой код обработчика уведомлений
	Edit_x0.EnableWindow(FALSE);
}


void CLogisticModelDlg::OnBnClickedCheckGenR()
{
	// TODO: добавьте свой код обработчика уведомлений
	Edit_R.EnableWindow(FALSE);
}


void CLogisticModelDlg::OnBnClickedRadioBifur()
{
	// TODO: добавьте свой код обработчика уведомлений
	Edit_x0.EnableWindow(FALSE);
	Edit_R.EnableWindow(FALSE);
	Edit_Length.EnableWindow(FALSE);

	Edit_M.EnableWindow(TRUE);
	Edit_Prec.EnableWindow(TRUE);

	GenerationX0.EnableWindow(FALSE);
	GenerationR.EnableWindow(FALSE);

	dots_draw.EnableWindow(TRUE);
	lines_draw.EnableWindow(TRUE);
}


void CLogisticModelDlg::OnBnClickedRadioLogistic()
{
	// TODO: добавьте свой код обработчика уведомлений
	Edit_M.EnableWindow(FALSE);
	Edit_Prec.EnableWindow(FALSE);

	Edit_x0.EnableWindow(TRUE);
	Edit_R.EnableWindow(TRUE);
	Edit_Length.EnableWindow(TRUE);
	GenerationX0.EnableWindow(TRUE);
	GenerationR.EnableWindow(TRUE);

	dots_draw.EnableWindow(FALSE);
	lines_draw.EnableWindow(FALSE);
}


BOOL CLogisticModelDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: добавьте свой код обработчика сообщений или вызов стандартного

	if (zDelta > 0) {
		scale *= 1.1;
	}
	else {
		if (scale > 1) scale /= 1.1;
	}


	if (m_radio_logistic.GetCheck() == BST_CHECKED) DrawSignal(sign, PicDc, Pic, &signal_pen, Length);
	if (m_radio_bifur.GetCheck() == BST_CHECKED)
	{
		if (dots_draw.GetCheck() == BST_CHECKED) DrawDotsBifurcation(BifurcationDots, PicDc, Pic, &bifurcation_dots_pen, bifurSize);
		if (lines_draw.GetCheck() == BST_CHECKED) DrawLinesBifurcation(BifurcationLines, PicDc, Pic, &bifurcation_lines_pen, bifurSize);
	}
	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}


void CLogisticModelDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: добавьте свой код обработчика сообщений или вызов стандартного
	if ((nFlags & MK_LBUTTON) == MK_LBUTTON)
	{
		defaultX0 += (point.x - prevX) * scale / 1000;
		prevX -= (point.x + prevX) * scale / 1000;


		defaultY0 += (point.y - prevY) * scale / 10000;
		prevY -= (point.y + prevY) * scale / 10000;

		needRedraw = true;
		needRedrawCount++;

		if (needRedrawCount % 2 == 0) {
			needRedraw = false;

			if (m_radio_logistic.GetCheck() == BST_CHECKED) DrawSignal(sign, PicDc, Pic, &signal_pen, Length);
			if (m_radio_bifur.GetCheck() == BST_CHECKED)
			{
				if (dots_draw.GetCheck() == BST_CHECKED) DrawDotsBifurcation(BifurcationDots, PicDc, Pic, &bifurcation_dots_pen, bifurSize);
				if (lines_draw.GetCheck() == BST_CHECKED) DrawLinesBifurcation(BifurcationLines, PicDc, Pic, &bifurcation_lines_pen, bifurSize);
			}
		}
	}
	else {
		prevX = point.x;
		prevY = point.y;

		if (needRedraw == true) {
			needRedraw = false;

			if (m_radio_logistic.GetCheck() == BST_CHECKED) DrawSignal(sign, PicDc, Pic, &signal_pen, Length);
			if (m_radio_bifur.GetCheck() == BST_CHECKED)
			{
				if (dots_draw.GetCheck() == BST_CHECKED) DrawDotsBifurcation(BifurcationDots, PicDc, Pic, &bifurcation_dots_pen, bifurSize);
				if (lines_draw.GetCheck() == BST_CHECKED) DrawLinesBifurcation(BifurcationLines, PicDc, Pic, &bifurcation_lines_pen, bifurSize);
			}
		}
	}

	CDialogEx::OnMouseMove(nFlags, point);
}
