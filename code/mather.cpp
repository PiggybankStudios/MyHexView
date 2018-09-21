/*
File:   mather.cpp
Author: Taylor Robbins
Date:   08\05\2018
Description: 
	** Mather is a playground for rendering math-related visualizations
*/

v2 ViewToSamplePos(v2 viewPos)
{
	return mather->sampleRec.topLeft + Vec2Multiply(Vec2Divide(viewPos - mather->viewRec.topLeft, mather->viewRec.size), mather->sampleRec.size);
}

v2 SampleToViewPos(v2 samplePos)
{
	return mather->viewRec.topLeft + Vec2Multiply(Vec2Divide(samplePos - mather->sampleRec.topLeft, mather->sampleRec.size), mather->viewRec.size);
}


// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void InitializeMatherState()
{
	mather->viewRec = NewRec(0, 0, RenderScreenSize.width*7/8, RenderScreenSize.height*7/8);
	mather->viewRec = RecSquarify(mather->viewRec);
	mather->viewRec.x = RenderScreenSize.width/2 - mather->viewRec.width/2;
	mather->viewRec.y = RenderScreenSize.height/2 - mather->viewRec.height/2;
	
	mather->sampleRec = NewRec(-Pi32*3, -4, Pi32*6, 8);
	mather->sampleRec.y += mather->sampleRec.height;
	mather->sampleRec.height = -mather->sampleRec.height;
	
	mather->initialized = true;
}

// +--------------------------------------------------------------+
// |                            Start                             |
// +--------------------------------------------------------------+
void StartMatherState(AppState_t fromState)
{
	
}

// +--------------------------------------------------------------+
// |                         Deinitialize                         |
// +--------------------------------------------------------------+
void DeinitializeMatherState()
{
	
	ClearPointer(mather);
}

r32 TestFunction1(r32 x)
{
	// return x;
	// r32 offset1 = 0;//-(((platform->programTime+50)%10000)/10000.0f)*2*Pi32;
	// r32 offset2 = 0;//(((platform->programTime+250)%400)/400.0f)*2*Pi32;
	// r32 offset3 = 0;//(((platform->programTime+750)%800)/800.0f)*2*Pi32;
	// r32 result = SinR32(offset1 + x/4)*5/4;// + SinR32(offset2 + x*1.4f)/4 + SinR32(offset3 + x*7.436f)/6;
	// result = AbsR32(result);
	// return result;
	// return AbsR32(SinR32(x) / PowR32(2, (PowR32(x, x) - Pi32/2) / Pi32));
	// return PowR32(2, 1/x);
	// return SinR32(SinR32( SinR32( SinR32( SinR32( SinR32( SinR32( SinR32( SinR32( SinR32(x))))))))));
	// return CosR32(2 * CosR32(2 * CosR32(2 * CosR32(2 * CosR32(2 * CosR32(2 * CosR32(2 * CosR32(2 * CosR32(2 * CosR32(x))))))))));
	return 2*PowR32(x, 5) + 32*PowR32(x, 4) + PowR32(x, 3) - 5*PowR32(x, 2) + 2*PowR32(x, 1);
}

// +--------------------------------------------------------------+
// |                      Update And Render                       |
// +--------------------------------------------------------------+
void UpdateAndRenderMatherState()
{
	if (!mather->grabbing)
	{
		mather->viewRec = NewRec(0, 0, RenderScreenSize.width*7/8, RenderScreenSize.height*7/8);
		mather->viewRec = RecSquarify(mather->viewRec);
		mather->viewRec.x = RenderScreenSize.width/2 - mather->viewRec.width/2;
		mather->viewRec.y = RenderScreenSize.height/2 - mather->viewRec.height/2;
	}
	
	// +--------------------------------------------------------------+
	// |                            Update                            |
	// +--------------------------------------------------------------+
	{
		if (ButtonPressed(MouseButton_Left) && IsInsideRec(mather->viewRec, RenderMousePos) && input->mouseInsideWindow)
		{
			mather->grabbing = true;
			mather->grabViewPos = RenderMousePos;
			mather->grabSampleTopLeft = mather->sampleRec.topLeft;
			DEBUG_WriteLine("Grabbed!");
		}
		if (ButtonDown(MouseButton_Left))
		{
			if (mather->grabbing && input->mouseInsideWindow)
			{
				v2 sampleScale = Vec2Divide(mather->viewRec.size, mather->sampleRec.size);
				v2 viewMoveAmount = RenderMousePos - mather->grabViewPos;
				v2 sampleMoveAmount = Vec2Divide(viewMoveAmount, sampleScale);
				mather->sampleRec.topLeft = mather->grabSampleTopLeft - sampleMoveAmount;
			}
		}
		else
		{
			if (mather->grabbing)
			{
				mather->grabbing = false;
				DEBUG_WriteLine("Released!");
			}
		}
	}
	
	// +--------------------------------------------------------------+
	// |                            Render                            |
	// +--------------------------------------------------------------+
	{
		// +==============================+
		// |            Setup             |
		// +==============================+
		Color_t backgroundColor = NewColor(Color_LightGrey);
		{
			RcBegin(&app->defaultShader, &app->defaultFont, NewRec(Vec2_Zero, RenderScreenSize));
			RcClearColorBuffer(backgroundColor);
			RcClearDepthBuffer(1.0f);
			RcEnableVignette(0.05f, 0.8f);
			RcDrawRectangle(NewRec(0, 0, RenderScreenSize.width, RenderScreenSize.height), NewColor(Color_DarkGray));
			RcDisableVignette();
			RcBindNewFont(&app->newFont);
			RcSetFontSize(12);
			RcSetFontAlignment(Alignment_Left);
			RcSetFontStyle(FontStyle_Default);
		}
		
		RcDrawButton(mather->viewRec, NewColor(Color_TransparentBlack), NewColor(Color_White), 1.0f);
		
		RcSetViewport(mather->viewRec);
		{
			//Draw X and Y Axis
			{
				v2 displayOrigin = SampleToViewPos(Vec2_Zero);//mather->viewRec.topLeft + Vec2Multiply(Vec2Divide(-mather->sampleRec.topLeft, mather->sampleRec.size), mather->viewRec.size);
				v2 clampedOrigin = NewVec2(
					ClampR32(displayOrigin.x, mather->viewRec.x+1, mather->viewRec.x + mather->viewRec.width-1),
					ClampR32(displayOrigin.y, mather->viewRec.y+1, mather->viewRec.y + mather->viewRec.height-1)
				);
				RcDrawLine(NewVec2(0, clampedOrigin.y), NewVec2(RenderScreenSize.width, clampedOrigin.y), 3, NewColor(Color_Red));
				RcDrawLine(NewVec2(clampedOrigin.x, 0), NewVec2(clampedOrigin.x, RenderScreenSize.height), 3, NewColor(Color_Green));
			}
			
			v2 lastPos = Vec2_Zero;
			r32 xOff = 0;
			do
			{
				r32 displayX = xOff + mather->viewRec.x;
				r32 sampleX = mather->sampleRec.x + (xOff / mather->viewRec.width) * mather->sampleRec.width;
				r32 sampleY = TestFunction1(sampleX);
				v2 displayPos = NewVec2(displayX, mather->viewRec.y + ((sampleY - mather->sampleRec.y) / mather->sampleRec.height) * mather->viewRec.height);
				if (xOff > 0)
				{
					RcDrawLine(lastPos, displayPos, 2.0f, NewColor(Color_Orange));
				}
				lastPos = displayPos;
				xOff += 1;
			} while (xOff < mather->viewRec.width);
			
		}
		RcSetViewport(NewRec(0, 0, RenderScreenSize.width, RenderScreenSize.height));
		
		if (IsInsideRec(mather->viewRec, RenderMousePos) && input->mouseInsideWindow)
		{
			RcSetFontSize(20);
			v2 sampleMousePos = ViewToSamplePos(RenderMousePos);
			r32 functionY = TestFunction1(sampleMousePos.x);
			v2 sampleFunctionPos = NewVec2(sampleMousePos.x, functionY);
			v2 viewFunctionPos = SampleToViewPos(sampleFunctionPos);
			
			if (viewFunctionPos.x >= mather->viewRec.x && viewFunctionPos.x <= mather->viewRec.x + mather->viewRec.width &&
				viewFunctionPos.y >= mather->viewRec.y && viewFunctionPos.y <= mather->viewRec.y + mather->viewRec.height)
			{
				RcDrawCircle(viewFunctionPos, 5, NewColor(Color_Cyan));
			}
			
			if (viewFunctionPos.y >= mather->viewRec.y && viewFunctionPos.y <= mather->viewRec.y + mather->viewRec.height)
			{
				v2 rightMarkPos = NewVec2(mather->viewRec.x + mather->viewRec.width, viewFunctionPos.y);
				RcDrawLine(viewFunctionPos, rightMarkPos, 1.0f, NewColor(Color_Yellow));
				v2 rightTextPos = rightMarkPos + NewVec2(5, RcGetMaxExtendUp() - RcGetLineHeight()/2);
				RcNewPrintString(rightTextPos, NewColor(Color_White), "y = %.02f", sampleFunctionPos.y);
			}
			
			if (viewFunctionPos.x >= mather->viewRec.x && viewFunctionPos.x <= mather->viewRec.x + mather->viewRec.width)
			{
				v2 bottomMarkPos = NewVec2(viewFunctionPos.x, mather->viewRec.y + mather->viewRec.height);
				RcDrawLine(viewFunctionPos, bottomMarkPos, 1.0f, NewColor(Color_Yellow));
				v2 bottomTextPos = bottomMarkPos + NewVec2(0, 5 + RcGetMaxExtendUp());
				RcSetFontAlignment(Alignment_Center);
				RcNewPrintString(bottomTextPos, NewColor(Color_White), "x = %.02f", sampleFunctionPos.x);
				RcSetFontAlignment(Alignment_Left);
			}
		}
	}
}

