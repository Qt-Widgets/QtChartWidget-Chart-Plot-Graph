
/*

 * myfft.cpp

 */

#include <stdio.h>

#include <stdlib.h>

#include <math.h>

#include "myfft.h"

#define PI 3.1415926

 /*------------���ն������һά������ɢ����Ҷ�任----------------------------------------

  * ԭ��

  *     һά��ɢ����Ҷ�任��ʽΪ�� X[k] = ��x[n]*e^(-2��i * nk/N)������n=0,1,2,...,N-1

  *     һά��ɢ����Ҷ��任��ʽΪ�� x[n] = 1/N * ��X[k]*e^(2��i * nk/N)������k=0,1,2,...,N-1

  *     ŷ����ʽ��e^ix = cos(x) + isin(x)

  *

  * ����1��in            �����һά��������

  * ����2��n1            �����һά��������ĳ���

  * ����3��out           �����һά��������

  * ����4��n2            �����һά��������ĳ���(�����n1���)

  * ����5��line_width    �����±�ļ��(Ĭ��Ϊ1)�������б任����1�������б任����һ�еĿ��

  * ����6��is_inv        �Ƿ�����任(Ĭ�ϲ���)

  *

  * ���� & 386520874@qq.com & 2016.01.25

  */

int myfft_by_define_c2c_1d(my_complex * in, int n1, my_complex * out, int n2, int line_width/*=1*/, int is_inv/*=0*/)

{
	if (n1 != n2) { return 0; }
	double _2PI_N = 2.0 * PI / n1;
	if (is_inv == 0) //���任
	{
		for (int m = 0; m < n1; m++)
		{

			int M = m * line_width;

			for (int k = 0; k < n1; k++)

			{

				int K = k * line_width;

				out[M].r += in[K].r * cos(_2PI_N * m * k) + in[K].i * sin(_2PI_N * m * k); //����ʵ��

				out[M].i += -in[K].r * sin(_2PI_N * m * k) + in[K].i * cos(_2PI_N * m * k); //�����鲿

			}

		}

	}
	else if (is_inv == 1) //��任

	{

		for (int m = 0; m < n1; m++)

		{

			int M = m * line_width;

			for (int k = 0; k < n1; k++)

			{

				int K = k * line_width;

				out[M].r += in[K].r * cos(_2PI_N * m * k) - in[K].i * sin(_2PI_N * m * k); //����ʵ��

				out[M].i += in[K].r * sin(_2PI_N * m * k) + in[K].i * cos(_2PI_N * m * k); //�����鲿

			}

			out[M].r /= n1; //��һ��

			out[M].i /= n1; //��һ��

		}

	}
	return 1;
}

//���ն�����ж�ά������ɢ����Ҷ�任

int myfft_by_define_c2c_2d(my_complex * in, int m, int n, int is_inv/*=0*/)

{

	my_complex * temp = new my_complex[m * n];

	memset(temp, 0, sizeof(my_complex) * m * n);



	for (int y = 0; y < n; y++) //���б任

	{

		myfft_by_define_c2c_1d(in + y * n, n, temp + y * n, n, 1, is_inv);

	}



	memset(in, 0, sizeof(my_complex) * m * n);

	for (int x = 0; x < m; x++) //���б任

	{

		myfft_by_define_c2c_1d(temp + x, m, in + x, m, n, is_inv);

	}



	delete[] temp; temp = NULL;



	return 1;

}

/*------------����һά������ɢ���ٸ���Ҷ�任(��2)----------------------------------------

 * ԭ��

 *     ����FFT�㷨��һ���ǻ�2�͵ģ���Ҫ�󱻱任�����鳤�ȱ�����2��m�ݴη���

 *     ����Ҫ����ڿ��̣���Ȼ����ڰ��ն�����м�����˵���ٶ��Ǻܿ�ġ����⣬

 *     ���л�4���㷨���Լ���ϻ��㷨�ȣ����������鳤��N���ϸ�����ơ�

 *     �������Լ�����һ���ʺ�����N��FFT�㷨�����ǱȽ��ѵģ�Ŀǰ���ǵ���FFTW����ʱ�պ����ðɡ�

 *

 * ����1��in            �����һά��������

 * ����2��n             ����/����һά��������ĳ���

 * ����3��line_width    �����±�ļ��(Ĭ��Ϊ1)�������б任����1�������б任����һ�еĿ��

 * ����4��is_inv        �Ƿ�����任(Ĭ�ϲ���)

 *

 * ���� & 386520874@qq.com & 2016.01.25

 */

int myfft_c2c_1d_base2(my_complex * in, int n, int line_width/*=1*/, int is_inv/*=0*/)

{

	int ttt = n;

	int L = 1; //��������Ĳ���

	while (ttt = ttt >> 1) { L++; }



	my_complex * X = in;



	int N = n;

	int len = 1 << L; //�������һά�������Ϊ N=2^L

	if (n * 2 == len) //�պ���2����

	{

		L -= 1;

	}
	else if (n < len) //˵��n����2����

	{

		//		printf("Erorr: n is not 2^m format.\n"); //�ǻ�2�㷨���д�����

		//		return 0;



		N = len;

		X = new my_complex[N]; //���������С

		int aaa = sizeof(my_complex);

		memset(X, 0, sizeof(my_complex) * N); //ȫ����ʼ��Ϊ0

		memcpy(X, in, sizeof(my_complex) * n); //��������

	}



	//--------���鵹λ�� ������(n0n1n2) => (n2n0n1)-------------------

	//���磺 3�Ķ�����Ϊ 011����Ӧ�ĵ�λ��Ϊ 110����Ϊ6

	my_complex * XX = new my_complex[N];

	for (int i = 0; i < N; i++)

	{

		int p = 0;

		for (int f = 0; f < L; f++)

		{

			if (i & (1 << f))

			{

				p += 1 << (L - f - 1);

			}

		}

		XX[i] = X[p];

	}

	memcpy(X, XX, sizeof(my_complex) * N); //��������

	delete[] XX; XX = NULL;



	//---------��ת����-------------------

	my_complex * W = new my_complex[N / 2];

	memset(W, 0, sizeof(my_complex) * N / 2); //ȫ����ʼ��Ϊ0

	for (int i = 0; i < N / 2; i++)

	{

		W[i] = my_complex(cos(-2.0 * PI * i / N), sin(-2.0 * PI * i / N));

	}



	//---------FFT�㷨(��2)----------------

	for (int f = 0; f < L; f++) //�����������

	{

		int G = 1 << (L - f);

		int num = 1 << f;



		for (int u = 0; u < num; u++) //ÿ���Ԫ�ظ���

		{

			for (int g = 0; g < G; g += 2) //ÿ�������

			{

				my_complex X1 = X[g * num + u];

				my_complex X2 = X[(g + 1) * num + u];



				X[g * num + u] = X1 + W[u * (1 << (L - f - 1))] * X2; //��������

				X[(g + 1) * num + u] = X1 - W[u * (1 << (L - f - 1))] * X2; //��������

			}

		}

	}



	if (X != NULL && X != in) //˵�������������ڴ�ռ�

	{

		memcpy(in, X, sizeof(my_complex) * n);

		if (X) { delete[] X; X = NULL; }

	}

	if (W != NULL) { delete[] W; W = NULL; }



	return 1;

}

int print_complex_array_1d(my_complex * in, int n)

{

	for (int i = 0; i < n; i++)

	{

		printf("%d:%f+i%f ", i, in[i].r, in[i].i);

	}

	printf("\n");



	return 1;

}

int print_complex_array_2d(my_complex * in, int m, int n)

{

	for (int y = 0; y < n; y++)

	{

		for (int x = 0; x < m; x++)

		{

			printf("%d_%d:%f+i%f ", y, x, (*(in + y * m + x)).r, (*(in + y * m + x)).i);

		}

		printf("\n");

	}

	printf("\n");



	return 1;

}
