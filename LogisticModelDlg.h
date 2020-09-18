
// LogisticModelDlg.h: файл заголовка
//

#pragma once


// Диалоговое окно CLogisticModelDlg
class CLogisticModelDlg : public CDialogEx
{
// Создание
public:
	CLogisticModelDlg(CWnd* pParent = nullptr);	// стандартный конструктор

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGISTICMODEL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// поддержка DDX/DDV


// Реализация
protected:
	HICON m_hIcon;

	// Созданные функции схемы сообщений
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:

	struct Points
	{
		double x_R;
		double y_x1000;
	};

	struct PointsToLine
	{
		double x_R;
		double y_x1000;
		int IndexPreviousPoint;
	};

	Points* BifurcationDots;
	PointsToLine* BifurcationLines;
	int bifurSize;

	CWnd* PicWnd;				//области рисования
	CDC* PicDc;
	CRect Pic;

	CPen osi_pen;				//ручки
	CPen setka_pen;
	CPen signal_pen;
	CPen bifurcation_dots_pen;
	CPen bifurcation_lines_pen;

	double Min, Max, Min1, Max1, Min2, Max2;
	double xx0, xxmax, yy0, yymax, xxi, yyi;
	char znach[1000];

	void Signal(int t);		//функция сигнала
	void DrawSignal(double*, CDC*, CRect, CPen*, double);
	void DrawDotsBifurcation(Points* Mass, CDC* WinDc, CRect WinPic, CPen* graphpen, int AbsMax);
	void DrawLinesBifurcation(PointsToLine* Mass, CDC* WinDc, CRect WinPic, CPen* graphpen, int AbsMax);
	void UniqArrayWithPrecision(double* arr, int size, int& NewSize);
	void Mashtab(double arr[], int dim, double* mmin, double* mmax);

	double* sign;			//глобальный массив для сигнала
	double* unicArr;
	int LengthR;

	double xp = 0, yp = 0,				//коэфициенты пересчета
		xmin = -Length * 0.1, xmax = Length,				//максисимальное и минимальное значение х 
		ymin = -0.001, ymax = 0.006;				//максисимальное и минимальное значение y

	double R_Step = 0.005;
	double R_from = 0.0;
	double R_to = 4.0;
	
	double scale = 1;
	bool needRedraw = false;
	int needRedrawCount = 0;

	double defaultX0 = 0;
	double defaultY0 = 0;
	double prevX = 0.0;
	double prevY = 0.0;

	double x0;
	double R_koef;
	int Length;
	afx_msg void OnBnClickedButtonExit();
	afx_msg void OnBnClickedButtonStart();
	virtual void OnOK();
	CButton GenerationX0;
	CEdit Edit_x0;
	afx_msg void OnBnClickedCheckGenX0();
	CButton GenerationR;
	CEdit Edit_R;
	afx_msg void OnBnClickedCheckGenR();
	CButton m_radio_logistic;
	CButton m_radio_bifur;
	CStatic GraphSignature;
	int num_of_counts;
	double precision;
	afx_msg void OnBnClickedRadioBifur();
	afx_msg void OnBnClickedRadioLogistic();
	CEdit Edit_Length;
	CEdit Edit_M;
	CEdit Edit_Prec;
	CButton dots_draw;
	CButton lines_draw;
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	double b1_dot;
	double b2_up_dot;
	double b2_down_dot;
};
