/*******************************************************************************
SPDX-License-Identifier: GPL-2.0-or-later
Copyright 2010-2025 Karlsruhe Institute of Technology (KIT)
Contact: stephan.allgeier∂kit.edu,
         Institute of Automation and Applied Informatics

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, 51 Franklin Street,
Fifth Floor, Boston, MA 02110-1301, USA.
*******************************************************************************/



#include "stdafx.h"
#include "ImageUndistortion.h"
#include <algorithm>
#include "interpolation.h"

CImageUndistortion::CImageUndistortion(EInterpolationMode eInterpolationModeX, EInterpolationMode eInterpolationModeY)
	:m_eInterpolationModeX(eInterpolationModeX)
	, m_eInterpolationModeY(eInterpolationModeY)
{
}

CUndistortedImage CImageUndistortion::UndistortImage(const StlImage<float>& Image, const CDenseMatrix& Distortion)
{
	ASSERT(Image.IsAllocated());
	ASSERT(Distortion.Rows() == static_cast<size_t>(Image.GetSize().y));
	ASSERT(Distortion.Cols() == 2);

	CUndistortedImage Undistorted;
	CRect Pos;

	auto X = Distortion.Col(0);
	auto Y = Distortion.Col(1);
	auto xOffset = ceil(X.Min());
	auto yOffset = ceil(Y.Min());
	X -= xOffset;
	Y -= yOffset;

	auto ImageSize = DetermineUndistortedImageSize(X, Y, CSize(static_cast<int>(Image.GetSize().x), static_cast<int>(Image.GetSize().y)));
	Undistorted.pUndistoredImage = std::make_shared<StlImage<float>>();
	Undistorted.pUndistoredImage->Alloc({ ImageSize.cx, ImageSize.cy });

	CreateUndistortedImage(Image, X, Y, m_eInterpolationModeX, m_eInterpolationModeY, *Undistorted.pUndistoredImage, Pos);

	Undistorted.ImageRowPositions = Distortion;
	Undistorted.TopLeftCorner.x = static_cast<ptrdiff_t>(xOffset);
	Undistorted.TopLeftCorner.y = static_cast<ptrdiff_t>(yOffset);
	return Undistorted;
}

CSize CImageUndistortion::DetermineUndistortedImageSize(const CDenseVector& XPositions, const CDenseVector& YPositions, const CSize& UndistortedImageSize) const
{
	auto xExtremes = XPositions.MinMax();
	auto minX = std::get<0>(xExtremes);
	auto maxX = std::get<1>(xExtremes);

	auto yExtremes = YPositions.MinMax();
	auto minY = std::get<0>(yExtremes);
	auto maxY = std::get<1>(yExtremes);

	return CSize(static_cast<int>(floor(maxX)) - static_cast<int>(ceil(minX)) + UndistortedImageSize.cx, static_cast<int>(floor(maxY)) - static_cast<int>(ceil(minY)) + 1);
}

/**	\brief Führt die Bewegungskorrektur für das Bild \a imgSrc durch.
*
*	\param[in] imgSrc Das Quellbild.
*	\param[in] vecPosX,vecPosY Vektoren mit den x- bzw. y-Koordinaten der
*		Zielpositionen der Bildzeilen von \a imgSrc.
*	\param[out] imgDest Das Zielbild; der Bildpuffer muss bereits in der
*		gewünschten Größe allokiert worden sein.
*	\param[out] posDest Die Position des linken oberen Eckpixels des belegten
*		Bereichs im Zielbild \a imgDest.
*
*	\author Stephan Allgeier
*/
void CImageUndistortion::CreateUndistortedImage(const StlImage<float>& imgSrc, const CDenseVector& vecPosX, const CDenseVector& vecPosY, EInterpolationMode eInterpolationModeX, EInterpolationMode eInterpolationModeY, StlImage<float>& imgDest, CRect& posDest)
{
	ASSERT(imgSrc.IsAllocated());
	ASSERT(imgDest.IsAllocated());
	ASSERT(vecPosX.Size() == vecPosY.Size());
	ASSERT(static_cast<INT_PTR>(vecPosX.Size()) == imgSrc.GetSize().y);

	StlImage<float> imgTemp;
	imgTemp.Alloc({ imgDest.GetSize().x, imgSrc.GetSize().y });

	if (eInterpolationModeX == EInterpolationMode::eLinear)
	{
		MotionCorrectionXLinear(imgSrc, vecPosX, imgTemp, posDest);
	}
	else if (eInterpolationModeX == EInterpolationMode::eCubicSpline)
	{
		MotionCorrectionXSpline(imgSrc, vecPosX, imgTemp, posDest);
	}

	if (eInterpolationModeY == EInterpolationMode::eLinear)
	{
		MotionCorrectionYLinear(imgTemp, vecPosX, vecPosY, imgSrc.GetSize().x, imgDest, posDest);
	}
	else if (eInterpolationModeY == EInterpolationMode::eCubicSpline)
	{
		MotionCorrectionYSpline(imgTemp, vecPosX, vecPosY, imgSrc.GetSize().x, imgDest, posDest);
	}
}

/**	\brief Korrigiert die Bewegungsartefakte in der horizontalen Dimension
*	für das Bild \a imgSrc durch lineare Interpolation.
*
*	\param[in] imgSrc Das Quellbild.
*	\param[in] vecPosX Ein Vektor mit den x-Koordinaten der Zielpositionen
*		der Bildzeilen von \a imgSrc bezüglich des Ausgabebilds \a imgDest.
*	\param[out] imgDest Das Zielbild; der Bildpuffer muss bereits in der
*		gewünschten Größe allokiert worden sein.
*	\param[out] posDest Die (rechteckige) Bounding Box des im Zielbild
*		\a imgDest belegten Bereichs.
*
*	\author Stephan Allgeier
*/
void CImageUndistortion::MotionCorrectionXLinear(const StlImage<float>& imgSrc, const CDenseVector& vecPosX, StlImage<float>& imgDest, CRect& posDest)
{
	ASSERT(imgSrc.IsAllocated());
	ASSERT(imgDest.IsAllocated());
	ASSERT(imgSrc.GetSize().y == static_cast<INT_PTR>(vecPosX.Size()));
	ASSERT(imgSrc.GetSize().y == imgDest.GetSize().y);

	long long  height = imgSrc.GetSize().y;
	long long  srcwidth = imgSrc.GetSize().x;
	long long  destwidth = imgDest.GetSize().x;

	// Initialize the destination image with 0
	imgDest.Clear(0.0f);

	posDest.left = std::max(lround(ceil(vecPosX.Min())), 0l);
	posDest.right = std::min(lround(floor(vecPosX.Max())) + static_cast<long>(srcwidth), static_cast<long>(destwidth));
	posDest.top = 0;
	posDest.bottom = static_cast<long>(height);

	for (long long y = 0; y < height; y++)
	{
		long long  dx1 = std::max(lround(ceil(vecPosX[y])), 0l);
		long long  dx2 = std::min(lround(floor(vecPosX[y])) + srcwidth, destwidth);

		// 0 <= qx < 1
		//
		// qx == 0, if the source pixels are perfectly aligned (horizontally)
		// with the destination pixel grid.
		//
		// qx is close to 0, if the source pixels are close to their RIGHT
		// neighbors in relation to the destination pixel grid; this means
		// that the destination pixels are close to their LEFT neighbors in
		// relation to the source pixel grid.
		//
		// qx is close to 1, if the source pixels are close to their LEFT
		// neighbors in relation to the destination pixel grid; this means
		// that the destination pixels are close to their RIGHT neighbors in
		// relation to the source pixel grid.
		double qx = ceil(vecPosX[y]) - vecPosX[y];

		// This should always be 0 in practice; the explicit initialization
		// makes it robust for all cases
		long long  sx = dx1 - lround(ceil(vecPosX[y]));

		// For integral vecPosX[y] (i.e. qx==0), sx will become srcwidth-1,
		// and therefore pSrc[y][sx+1] can potentially refer to outside the
		// array bounds!
		if (qx != 0)
		{
			for (long long dx = dx1; dx < dx2; dx++)
			{
				double fDestValue = (1 - qx) * imgSrc[{sx, y}] + qx * imgSrc[{sx + 1, y}];
				imgDest[{dx, y}] = static_cast<float>(fDestValue);
				sx++;
			}
		}
		else
		{
			for (long long dx = dx1; dx < dx2; dx++)
			{
				imgDest[{dx, y}] = imgSrc[{sx++, y}];
			}
		}
	}

	//imgDest.Put2d(0, 0, destwidth, height, reinterpret_cast<unsigned char *>(pDestBuffer.data()));
}

/**	\brief Korrigiert die Bewegungsartefakte in der horizontalen Dimension
*	für das Bild \a imgSrc durch (kubische) Spline-Interpolation.
*
*	\param[in] imgSrc Das Quellbild.
*	\param[in] vecPosX Ein Vektor mit den x-Koordinaten der Zielpositionen
*		der Bildzeilen von \a imgSrc bezüglich des Ausgabebilds \a imgDest.
*	\param[out] imgDest Das Zielbild; der Bildpuffer muss bereits in der
*		gewünschten Größe allokiert worden sein.
*	\param[out] posDest Die (rechteckige) Bounding Box des im Zielbild
*		\a imgDest belegten Bereichs.
*
*	\author Stephan Allgeier
*/
void CImageUndistortion::MotionCorrectionXSpline(const StlImage<float>& imgSrc, const CDenseVector& vecPosX, StlImage<float>& imgDest, CRect& posDest)
{
	ASSERT(imgSrc.IsAllocated());
	ASSERT(imgDest.IsAllocated());
	ASSERT(vecPosX.Size() == vecPosX.Size());
	ASSERT(imgSrc.GetSize().y == imgDest.GetSize().y);

	long long  height = imgSrc.GetSize().y;
	long long  srcwidth = imgSrc.GetSize().x;
	long long  destwidth = imgDest.GetSize().x;

	// Buffers for the source and destination image data
	float* pSrcBuffer = new float[srcwidth * height];
	float* pDestBuffer = new float[destwidth * height];

	// Pointer arrays for convenient access to the source and destination
	// buffers by x and y coordinates. Take care to use the y coordinate
	// first, i.e. pSrc[y][x] and pDest[y][x].
	/*float** pSrc = new float*[height];
	for (size_t y = 0; y < height; y++)
	{
		pSrc[y] = &(pSrcBuffer[y * srcwidth]);
	}

	float** pDest = new float*[height];
	for (size_t y = 0; y < height; y++)
	{
		pDest[y] = &(pDestBuffer[y * destwidth]);
	}*/

	// Initialize the temporary and destination image buffers with 0
	imgDest.Clear(0.0);

	//imgSrc.Get2d(0, 0, srcwidth, height, reinterpret_cast<unsigned char *>(pSrcBuffer), srcwidth * height);
	//imgDest.Get2d(0, 0, destwidth, height, reinterpret_cast<unsigned char *>(pDestBuffer), destwidth * height);

	posDest.left = std::max(lround(ceil(vecPosX.Min())), 0l);
	posDest.right = std::min(lround(floor(vecPosX.Max())) + static_cast<long>(srcwidth), static_cast<long>(destwidth));
	posDest.top = 0;
	posDest.bottom = static_cast<long>(height);

	alglib::real_1d_array outval;
	alglib::real_1d_array outind;
	alglib::real_1d_array inval;
	alglib::real_1d_array inind;

	outval.setlength(destwidth);
	outind.setlength(destwidth);
	inval.setlength(srcwidth);
	inind.setlength(srcwidth);

	for (long long y = 0; y < height; y++)
	{
		// The range of destination x-coordinates to be calculated for the
		// current row. Note that dx1 is the first pixel at the left end
		// INSIDE the range, while dx2 is the first pixel at the right end
		// OUTSIDE the range.
		long long  dx1 = std::max(lround(ceil(vecPosX[y])), 0l);
		long long  dx2 = std::min(lround(floor(vecPosX[y])) + srcwidth, destwidth);
		long long  dwidth = dx2 - dx1;

		for (long long sx = 0; sx < srcwidth; sx++)
		{
			inind[sx] = vecPosX[y] + sx;
			inval[sx] = imgSrc[{sx, y}];
		}

		for (long long dx = 0; dx < dwidth; dx++)
		{
			outind[dx] = static_cast<double>(dx1 + dx);
		}

		alglib::spline1dconvcubic(inind, inval, srcwidth, 0, 0, 0, 0, outind, dwidth, outval);

		for (long long dx = 0; dx < dwidth; dx++)
		{
			imgDest[{dx1 + dx, y}] = static_cast<float>(outval[dx]);
		}
	}

	/*imgDest.Put2d(0, 0, destwidth, height, reinterpret_cast<unsigned char *>(pDestBuffer));

	delete[] pSrc;
	delete[] pDest;*/
	delete[] pSrcBuffer;
	delete[] pDestBuffer;
}

/**	\brief Korrigiert die Bewegungsartefakte in der vertikalen Dimension für
*	das Bild \a imgSrc durch lineare Interpolation.
*
*	\param[in] imgSrc Das Quellbild.
*	\param[in] vecPosX,vecPosY Vektoren mit den x- bzw. y-Koordinaten der
*		Zielpositionen der Bildzeilen von \a imgSrc bezüglich des
*		Ausgabebilds \a imgDest.
*	\param[in] nFrameWidth Die Breite einer Bildzeile.
*	\param[out] imgDest Das Zielbild; der Bildpuffer muss bereits in der
*		gewünschten Größe allokiert worden sein.
*	\param[out] posDest Die (rechteckige) Bounding Box des im Zielbild
*		\a imgDest belegten Bereichs.
*
*	\author Stephan Allgeier
*/
void CImageUndistortion::MotionCorrectionYLinear(const StlImage<float>& imgSrc, const CDenseVector& vecPosX, const CDenseVector& vecPosY, size_t nFrameWidth, StlImage<float>& imgDest, CRect& posDest)
{
	ASSERT(imgSrc.IsAllocated());
	ASSERT(imgDest.IsAllocated());
	ASSERT(vecPosX.Size() == vecPosY.Size());
	ASSERT(imgSrc.GetSize().y == static_cast<INT_PTR>(vecPosY.Size()));
	ASSERT(imgSrc.GetSize().x == imgDest.GetSize().x);

	long long  width = imgSrc.GetSize().x;
	long long  srcheight = imgSrc.GetSize().y;
	long long  destheight = imgDest.GetSize().y;

	// Buffers for the source and destination image data
	float* pSrcBuffer = new float[width * srcheight];
	float* pDestBuffer = new float[width * destheight];

	// Pointer arrays for convenient access to the source and destination
	// buffers by x and y coordinates. Take care to use the y coordinate
	// first, i.e. pSrc[y][x] and pDest[y][x].
	/*float** pSrc = new float*[srcheight];
	for (size_t y = 0; y < srcheight; y++)
	{
		pSrc[y] = &(pSrcBuffer[y * width]);
	}

	//float** pDest = new float*[destheight];
	for (size_t y = 0; y < destheight; y++)
	{
		pDest[y] = &(pDestBuffer[y * width]);
	}
	*/
	// Initialize the destination image buffer with 0
	imgDest.Clear(0.0);

	//imgSrc.Get2d(0, 0, width, srcheight, reinterpret_cast<unsigned char *>(pSrcBuffer), width * srcheight);
	//imgDest.Get2d(0, 0, width, destheight, reinterpret_cast<unsigned char *>(pDestBuffer), width * destheight);

	posDest.left = std::max(lround(ceil(vecPosX.Min())), 0l);
	posDest.right = std::min(lround(floor(vecPosX.Max())) + static_cast<long>(nFrameWidth), static_cast<long>(width));
	posDest.top = std::max(lround(ceil(vecPosY.Min())), 0l);
	posDest.bottom = std::min(lround(floor(vecPosY.Max())) + 1, static_cast<long>(destheight));

	for (long long sy2 = 1; sy2 < srcheight; sy2++)
	{
		long long  sy1 = sy2 - 1;

		//ASSERT(vecPosY[sy1] < vecPosY[sy2]);

		// The range of destination x- and y-coordinates to be interpolated
		// between the current two source rows, sy1 and sy2. Note that dx1
		// and dy1 are the first pixels at the left and top ends INSIDE the
		// range, while dx2 and dy2 are the first pixels at the right and
		// bottom ends OUTSIDE the range.
		long long  dy1 = std::max(lround(ceil(vecPosY[sy1])), 0l);
		long long  dy2 = std::min(static_cast<long long>(lround(floor(vecPosY[sy2]))) + 1u, destheight);
		long long  dx1 = std::max(lround(ceil(std::max(vecPosX[sy1], vecPosX[sy2]))), 0l);
		long long  dx2 = std::min(static_cast<long long>(lround(floor(std::min(vecPosX[sy1], vecPosX[sy2]))) + nFrameWidth), width);

		// This loop iterates over all destination image rows located between
		// the positions of the source image rows sy1 and sy2, including the
		// boundary rows if they are located at an integral y-coordinate
		// (which practically never occurs). If the boundary sy2 is included
		// (as dy2) in the loop, then it will be included again as sy1 in the
		// next iteration of the outer loop and that destination image row
		// will be written twice. This tiny performance issue buys us the
		// most elegant handling of integral y-coordinates of source image
		// rows (especially concerning the very first and/or last source
		// image row) that I could think of.
		for (long long dy = dy1; dy < dy2; dy++)
		{
			// 0 <= qy <= 1
			//
			// qy == 0, if dy == vecPosY[sy1].
			// qy == 1, if dy == vecPosY[sy2].
			// 0 < qy < 1, if vecPosY[sy1] < dy < vecPosY[sy2].
			// qy is close to 0, if dy is close to vecPosY[sy1].
			// qy is close to 1, if dy is close to vecPosY[sy2].
			double qy = (dy - vecPosY[sy1]) / (vecPosY[sy2] - vecPosY[sy1]);

			for (long long x = dx1; x < dx2; x++)
			{
				double fDestValue = (1 - qy) * imgSrc[{x, sy1}] + qy * imgSrc[{x, sy2}];
				imgDest[{x, dy}] = static_cast<float>(fDestValue);
			}
		}
	}

	/*imgDest.Put2d(0, 0, width, destheight, reinterpret_cast<unsigned char *>(pDestBuffer));

	delete[] pSrc;
	delete[] pDest;*/
	delete[] pSrcBuffer;
	delete[] pDestBuffer;
}

/**	\brief Korrigiert die Bewegungsartefakte in der vertikalen Dimension für
*	das Bild \a imgSrc durch (kubische) Spline-Interpolation.
*
*	\param[in] imgSrc Das Quellbild.
*	\param[in] vecPosX,vecPosY Vektoren mit den x- bzw. y-Koordinaten der
*		Zielpositionen der Bildzeilen von \a imgSrc bezüglich des
*		Ausgabebilds \a imgDest.
*	\param[in] nFrameWidth Die Breite einer Bildzeile.
*	\param[out] imgDest Das Zielbild; der Bildpuffer muss bereits in der
*		gewünschten Größe allokiert worden sein.
*	\param[out] posDest Die (rechteckige) Bounding Box des im Zielbild
*		\a imgDest belegten Bereichs.
*
*	\author Stephan Allgeier
*/
void CImageUndistortion::MotionCorrectionYSpline(const StlImage<float>& imgSrc, const CDenseVector& vecPosX, const CDenseVector& vecPosY, size_t nFrameWidth, StlImage<float>& imgDest, CRect& posDest)
{
	ASSERT(imgSrc.IsAllocated());
	ASSERT(imgDest.IsAllocated());
	ASSERT(vecPosX.Size() == vecPosY.Size());
	ASSERT(imgSrc.GetSize().y == static_cast<INT_PTR>(vecPosY.Size()));
	ASSERT(imgSrc.GetSize().x == imgDest.GetSize().x);

	long long  width = imgSrc.GetSize().x;
	long long  srcheight = imgSrc.GetSize().y;
	long long  destheight = imgDest.GetSize().y;

	// Buffers for the source and destination image data
	float* pSrcBuffer = new float[width * srcheight];
	float* pDestBuffer = new float[width * destheight];

	// Pointer arrays for convenient access to the source and destination
	// buffers by x and y coordinates. Take care to use the y coordinate
	// first, i.e. pSrc[y][x] and pDest[y][x].
	/*float** pSrc = new float*[srcheight];
	for (size_t y = 0; y < srcheight; y++)
	{
		pSrc[y] = &(pSrcBuffer[y * width]);
	}

	float** pDest = new float*[destheight];
	for (size_t y = 0; y < destheight; y++)
	{
		pDest[y] = &(pDestBuffer[y * width]);
	}
	*/
	// Initialize the destination image buffer with 0
	imgDest.Clear(0.0);

	//imgSrc.Get2d(0, 0, width, srcheight, reinterpret_cast<unsigned char *>(pSrcBuffer), width * srcheight);
	//imgDest.Get2d(0, 0, width, destheight, reinterpret_cast<unsigned char *>(pDestBuffer), width * destheight);

	posDest.left = std::max(lround(ceil(vecPosX.Min())), 0l);
	posDest.right = std::min(lround(floor(vecPosX.Max())) + static_cast<long>(nFrameWidth), static_cast<long>(width));
	posDest.top = std::max(lround(ceil(vecPosY.Min())), 0l);
	posDest.bottom = std::min(lround(floor(vecPosY.Max())) + 1, static_cast<long>(destheight));

	alglib::real_1d_array outval;
	alglib::real_1d_array outind;
	alglib::real_1d_array inval;
	alglib::real_1d_array inind;

	outval.setlength(destheight);
	outind.setlength(destheight);
	inval.setlength(srcheight);
	inind.setlength(srcheight);

	// The range of destination x- and y-coordinates of the motion-corrected
	// image. Note that x1 and y1 are the first pixels at the left and top
	// ends INSIDE the range, while x2 and y2 are the first pixels at the
	// right and bottom ends OUTSIDE the range.
	long long  x1 = posDest.left;
	long long  x2 = posDest.right;
	long long  dy1 = posDest.top;
	long long  dy2 = posDest.bottom;
	long long  dheight = dy2 - dy1;

	for (long long x = x1; x < x2; x++)
	{
		for (long long sy = 0; sy < srcheight; sy++)
		{
			inind[sy] = vecPosY[sy];
			inval[sy] = imgSrc[{x, sy}];
		}

		for (long long dy = 0; dy < dheight; dy++)
		{
			outind[dy] = static_cast<double>(dy1 + dy);
		}

		alglib::spline1dconvcubic(inind, inval, srcheight, 0, 0, 0, 0, outind, dheight, outval);

		for (long long dy = 0; dy < dheight; dy++)
		{
			imgDest[{x, dy1 + dy}] = static_cast<float>(outval[dy]);
		}
	}

	/*imgDest.Put2d(0, 0, width, destheight, reinterpret_cast<unsigned char *>(pDestBuffer));

	delete[] pSrc;
	delete[] pDest;*/
	delete[] pSrcBuffer;
	delete[] pDestBuffer;
}

CDenseMatrix CImageUndistortion::InterpolateRowPositions(const CDenseMatrix& SubImagePositions, size_t nSubImageHeight)
{
	CDenseMatrix InterpolatedPos(SubImagePositions.Rows() * nSubImageHeight, SubImagePositions.Cols());
	SubImagePositions.CubicSplineColInterpolation(nSubImageHeight, InterpolatedPos);
	return InterpolatedPos;
}