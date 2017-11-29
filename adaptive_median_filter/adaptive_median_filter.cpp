#include <iostream>
#include <fstream>
#include <Windows.h>
#include <algorithm>

using namespace std;

void MemFree2D(unsigned char **Mem, int nHeight);
unsigned char** MemAlloc2D(int nHeight, int nWidth, unsigned char nInitVal);
void InputSaltPepperNoise(unsigned char** In, unsigned char** Out, int nHeight, int nWidth, float fSProb, float fPProb);
unsigned char** Padding(unsigned char** In, int nHeight, int nWidth, int nFilterSize);
void MedianFilter(unsigned char **In_Pad, unsigned char **Out, int nHeight, int nWidth, int nFilterSize);
void AdaptiveMedianFilter(unsigned char **In, unsigned char **Out, int nHeight, int nWidth, int nFilterSize_Min, int nFilterSize_Max);

int main()
{
	int width = 512;
	int height = 512;

	int nFilterSize;

	unsigned char **input_img;
	unsigned char **out_img;
	unsigned char **out_img_Median;
	unsigned char **Pad_img;

	cin >> nFilterSize;

	FILE *infile;
	fopen_s(&infile, "circuit512.raw", "rb");

	input_img = MemAlloc2D(width, height, 0);
	out_img = MemAlloc2D(width, height, 0);
	out_img_Median = MemAlloc2D(width, height, 0);

	for (int h = 0; h < height; h++)
	{
		fread(input_img[h], sizeof(unsigned char), width, infile);
	}

	InputSaltPepperNoise(input_img, out_img, height, width, 0.25, 0);
	//AdaptiveMedianFilter(input_img, out_img, height, width, 3, nFilterSize);
	//MedianFilter(input_img, out_img_Median, height, width, nFilterSize);

	MemFree2D(input_img, 512);
	MemFree2D(out_img, 512);
	MemFree2D(out_img_Median, 512);

	fclose(infile);

	return 0;
}

void MemFree2D(unsigned char **Mem, int nHeight)
{
	for (int n = 0; n < nHeight; n++)
	{
		delete[] Mem[n];
	}
	delete[] Mem;
}

unsigned char** MemAlloc2D(int nHeight, int nWidth, unsigned char nInitVal)
{
	unsigned char** rtn = new unsigned char*[nHeight];
	for (int n = 0; n < nHeight; n++)
	{
		rtn[n] = new unsigned char[nWidth];
		memset(rtn[n], nInitVal, sizeof(unsigned char) * nWidth);
	}
	return rtn;
}

unsigned char** Padding(unsigned char** In, int nHeight, int nWidth, int nFilterSize)
{
	int nPadSize = (int)(nFilterSize / 2);
	unsigned char** Pad = MemAlloc2D(nHeight + 2 * nPadSize, nWidth + 2 * nPadSize, 0);

	for (int h = 0; h < nHeight; h++)
	{
		for (int w = 0; w < nWidth; w++)
		{
			Pad[h + nPadSize][w + nPadSize] = In[h][w];
		}
	}

	for (int h = 0; h < nPadSize; h++)
	{
		for (int w = 0; w < nWidth; w++)
		{
			Pad[h][w + nPadSize] = In[0][w];
			Pad[h + nHeight - 1][w + nPadSize] = In[nHeight - 1][w];
		}
	}

	for (int h = 0; h < nHeight; h++)
	{
		for (int w = 0; w < nPadSize; w++)
		{
			Pad[h + nPadSize][w] = In[h][0];
			Pad[h + nPadSize][w + nWidth - 1] = In[h][nWidth - 1];
		}
	}

	for (int h = 0; h < nPadSize; h++)
	{
		for (int w = 0; w < nPadSize; w++)
		{
			Pad[h][w] = In[0][0];
			Pad[h + nHeight - 1][w] = In[nHeight - 1][0];
			Pad[h][w + nWidth - 1] = In[0][nWidth - 1];
			Pad[h + nHeight - 1][w + nWidth - 1] = In[nHeight - 1][nWidth - 1];
		}
	}

	return Pad;
}

void InputSaltPepperNoise(unsigned char** In, unsigned char** Out, int nHeight, int nWidth, float fSProb, float fPProb)
{
	FILE *out_sp_noise;
	fopen_s(&out_sp_noise, "saltpepper_noise.raw", "wb");

	float Low = fSProb;
	float High = 1.0f - fPProb;
	float fRand;

	srand(GetTickCount());

	for (int h = 0; h < nHeight; h++)
	{
		for (int w = 0; w < nWidth; w++)
		{
			fRand = ((float)rand() / RAND_MAX);

			if (fRand < Low)
			{
				Out[h][w] = 255;
			}
			else if (fRand > High)
			{
				Out[h][w] = 0;
			}
			else Out[h][w] = In[h][w];
		}
	}

	for (int h = 0; h < nHeight; h++)
	{
		fwrite(Out[h], sizeof(unsigned char), nWidth, out_sp_noise);
	}

	fclose(out_sp_noise);
}

void MedianFilter(unsigned char **In_Pad, unsigned char **Out, int nHeight, int nWidth, int nFilterSize)
{
	BYTE *arr;
	arr = new BYTE[nFilterSize*nFilterSize];

	In_Pad = Padding(In_Pad, nHeight, nWidth, nFilterSize);

	FILE *out_Median;
	fopen_s(&out_Median, "median.raw", "wb");

	for (int h = 0; h < nHeight; h++)
	{
		for (int w = 0; w < nWidth; w++)
		{
			int i = 0;
			for (int n = 0; n < nFilterSize; n++)
			{
				for (int m = 0; m< nFilterSize; m++)
				{
					arr[i] = In_Pad[h + n][w + m];
					i++;
				}
			}
			sort(arr, arr + nFilterSize*nFilterSize);

			Out[h][w] = arr[(int)(nFilterSize*nFilterSize / 2)];
		}
	}

	for (int h = 0; h < nHeight; h++)
	{
		fwrite(Out[h], sizeof(unsigned char), nWidth, out_Median);
	}

	fclose(out_Median);
}

void AdaptiveMedianFilter(unsigned char **In, unsigned char **Out, int nHeight, int nWidth, int nFilterSize_Min, int nFilterSize_Max)
{
	BYTE *arr;
	arr = new BYTE[nFilterSize_Max*nFilterSize_Max];
	int nFilter_Change = nFilterSize_Min;

	int PadSize = nFilterSize_Min / 2;

	int mid;
	int A1, A2;
	int Zxy;
	int B1, B2;
	int result;

	FILE *out_Adpat_Median;
	fopen_s(&out_Adpat_Median, "adpat_median.raw", "wb");

	unsigned char ** In_Pad = Padding(In, nHeight, nWidth, 3);

	for (int h = 0; h < nHeight; h++)
	{
		for (int w = 0; w < nWidth; w++)
		{
			A1 = 0;
			A2 = 0;
			Zxy = In_Pad[h + 1][w + 1];
			B1 = 0;
			B2 = 0;
			result = 0;

			nFilter_Change = nFilterSize_Min;

			while (nFilter_Change <= nFilterSize_Max)
			{
				int i = 0;

				if (h + nFilter_Change > 513)
				{
					for (int n = 0; n < nFilterSize_Min; n++)
					{
						for (int m = 0; m < nFilter_Change; m++)
						{
							arr[i] = In_Pad[h + n][w + m];
							i++;
						}
					}
					sort(arr, arr + nFilter_Change*nFilterSize_Min);

					mid = (nFilter_Change*nFilterSize_Min) / 2;

					A1 = arr[mid] - arr[0];
					A2 = arr[mid] - arr[nFilter_Change*nFilterSize_Min - 1];
				}

				else if (w + nFilter_Change > 513)
				{
					for (int n = 0; n < nFilter_Change; n++)
					{
						for (int m = 0; m < nFilterSize_Min; m++)
						{
							arr[i] = In_Pad[h + n][w + m];
							i++;
						}
					}
					sort(arr, arr + nFilter_Change*nFilterSize_Min);

					mid = (nFilter_Change*nFilterSize_Min) / 2;

					A1 = arr[mid] - arr[0];
					A2 = arr[mid] - arr[nFilter_Change*nFilterSize_Min - 1];
				}

				else
				{
					for (int n = 0; n < nFilter_Change; n++)
					{
						for (int m = 0; m < nFilter_Change; m++)
						{
							arr[i] = In_Pad[h + n][w + m];
							i++;
						}
					}

					sort(arr, arr + nFilter_Change*nFilter_Change);

					mid = (nFilter_Change*nFilter_Change) / 2;

					A1 = arr[mid] - arr[0];
					A2 = arr[mid] - arr[nFilter_Change*nFilter_Change - 1];
				}

				if (A1 >0 && A2 <0)
				{
					break;
				}

				else
				{
					nFilter_Change += 2;
				}
			}

			if (nFilter_Change > nFilterSize_Max)
			{
				result = arr[mid];
			}

			else
			{
				B1 = Zxy - arr[0];
				B2 = Zxy - arr[nFilter_Change*nFilter_Change - 1];

				if (B1 >0 && B2 <0)
				{
					result = Zxy;

				}

				else
				{
					result = arr[mid];
				}
			}
			//cout << h << "   " << w << endl;

			Out[h][w] = (unsigned char)result;
		}
	}

	for (int h = 0; h < nHeight; h++)
	{
		fwrite(Out[h], sizeof(unsigned char), nWidth, out_Adpat_Median);
	}

	fclose(out_Adpat_Median);
}