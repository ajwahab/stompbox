#include "SerialDisplayInterface.h"

#ifndef _WIN32
#include "unistd.h"
#endif

bool SerialDisplayInterface::Connect()
{
	fprintf(stderr, "Connect to serial display\n");

	serial = new Serial(SERIAL_INTERFACE_PORT);

	if (!serial->IsConnected())
	{
		delete serial;

		return false;
	}

	serialTFT = new SerialTFT(serial);

	isConnected = true;

	ResetDisplay();

	//ChangePreset("Started");

#ifndef _WIN32
	sleep(1);
#endif

	fprintf(stderr, "Serial display connected\n");

	return true;
}

void SerialDisplayInterface::Close()
{
	delete serial;
	delete serialTFT;
}

void SerialDisplayInterface::ResetDisplay()
{
	serialTFT->fillScreen(BACKGROUND_COLOR);
	serialTFT->updateScreen();
}

void IntToRGB(uint32_t colorRGB, uint8_t* r, uint8_t* g, uint8_t* b)
{
	*r = (uint8_t)((colorRGB >> 16) & 0xFF);
	*g = (uint8_t)((colorRGB >> 8) & 0xFF);
	*b = (uint8_t)(colorRGB & 0xFF);
}

uint16_t ColorRGBto565(uint8_t r, uint8_t g, uint8_t b)
{
	return (((31 * (r + 4)) / 255) << 11) |
		(((63 * (g + 2)) / 255) << 5) |
		((31 * (b + 4)) / 255);
}

int patchBoxStartY = 0;
int patchBoxHeight = 50;

void SerialDisplayInterface::SetPresetText(const char* presetName, const char* textColor)
{
	uint8_t r, g, b;
	uint16_t color565;

	IntToRGB(strtol(textColor + 1, NULL, 16), &r, &g, &b);

	color565 = ColorRGBto565(r, g, b);

	serialTFT->fillRect(0, patchBoxStartY, 320, patchBoxHeight, BACKGROUND_COLOR);

	serialTFT->setTextColor(color565);
	serialTFT->setTextAlignment("mc");
	serialTFT->setTextFont("FSSB18");

	serialTFT->drawText(160, patchBoxStartY + (patchBoxHeight / 2), presetName);

	serialTFT->updateScreen();
}

int stompBoxHeight = 70;
int stompBoxStartY = 240 - stompBoxHeight;

void SerialDisplayInterface::HandleStomp(int stompNumber, bool enabled, const char* pluginName, const char* parameterName, const char* pluginBGColor, const char* pluginFGColor)
{
	uint8_t r, g, b;
	uint16_t bg565, fg565;

	IntToRGB(strtol(pluginBGColor + 1, NULL, 16), &r, &g, &b);

	if (!enabled)
	{
		r /= 3;
		g /= 3;
		b /= 3;
	}

	bg565 = ColorRGBto565(r, g, b);

	IntToRGB(strtol(pluginFGColor + 1, NULL, 16), &r, &g, &b);

	if (!enabled)
	{
		r /= 3;
		g /= 3;
		b /= 3;
	}

	fg565 = ColorRGBto565(r, g, b);

	int stompWidth = screenWidth / 3;

	serialTFT->fillRect(stompWidth * stompNumber, stompBoxStartY, stompWidth, stompBoxHeight, bg565);
	serialTFT->drawRect(stompWidth * stompNumber, stompBoxStartY, stompWidth, stompBoxHeight, fg565);

	serialTFT->setTextColor(fg565);
	serialTFT->setTextAlignment("mc");

	int xCenter = stompWidth * (stompNumber + 0.5);

	serialTFT->setTextFont("FSSB12");

	//int textWidth = max(screen.textWidth(text1), screen.textWidth(text2));

	//if (textWidth > (stompWidth - 4))
	//{
	//	screen.setFreeFont(FSSB9);
	//}

	serialTFT->drawText(xCenter, stompBoxStartY + (stompBoxHeight / 4), pluginName);
	serialTFT->drawText(xCenter, stompBoxStartY + (stompBoxHeight * .75), parameterName);

	serialTFT->updateScreen();
}


int tunerHeight = 30;
int tunerCenterWidth = 35;
int tunerDeltaWidth = 10;

int lastTunerNote = 0;
int lastTunerDelta = -1000;

void SerialDisplayInterface::ResetTuner()
{
	int yCenter = screenHeight / 2;

	serialTFT->fillRect(0, yCenter - tunerHeight, screenWidth, tunerHeight * 2, BACKGROUND_COLOR);
	serialTFT->updateScreen();

	histSize = 0;
}

void SerialDisplayInterface::UpdateTuner(double frequency)
{
	if (frequency < 1)
	{
		histSize--;

		if (histSize <= 0)
		{
			ResetTuner();
		}
	}
	else
	{
		runningFrequency = frequency; // ((histSize * runningFrequency) + ((maxHistSize - histSize) * frequency)) / maxHistSize;

		histSize++;

		if (histSize > (maxHistSize - 1))
			histSize = (maxHistSize - 1);

		int midiNote = round(log(runningFrequency / 440.0) / log(2) * 12) + 69;

		float targetFreq = 440.0 * pow(2.0, ((double)midiNote - 69) / 12);

		//fprintf(stderr, "Tuner frequency: %f Target frequency: %f Running Frequency: %f\n", frequency, targetFreq, runningFrequency);

		double centDelta = 1200 * (log(runningFrequency / targetFreq) / log(2));

		bool isNegative = centDelta < 0;

		centDelta = abs(centDelta);

		int intDelta = (int)centDelta;

		uint8_t tint = (uint8_t)((centDelta - intDelta) * 255.0);

		//fprintf(stderr, "Midi Note: %d centDelta: %f\n", midiNote, centDelta);

		if ((intDelta == lastTunerDelta) && (midiNote == lastTunerNote))
		{
		}
		else
		{
			lastTunerDelta = intDelta;
			lastTunerNote = midiNote;

			int xCenter = screenWidth / 2;
			int yCenter = screenHeight / 2;

			serialTFT->fillRect(0, yCenter - tunerHeight, screenWidth, tunerHeight * 2, BACKGROUND_COLOR);

			if (intDelta == 0)
			{
				serialTFT->fillRect(xCenter - tunerCenterWidth, yCenter - tunerHeight, tunerCenterWidth * 2, tunerHeight * 2, TFT_BLUE);
			}
			else
			{
				if (isNegative)
				{
					uint16_t color = ColorRGBto565(tint, 0, 0);

					for (int i = 0; i < intDelta; i++)
					{
						serialTFT->fillRect(xCenter - tunerCenterWidth + (tunerDeltaWidth * (-i - 1) * 2),
							yCenter - tunerHeight, (tunerDeltaWidth - 2) * 2, tunerHeight * 2, color);
					}
				}
				else
				{
					uint16_t color = ColorRGBto565(tint, tint, 0);

					for (int i = 0; i < intDelta; i++)
					{
						serialTFT->fillRect(xCenter + tunerCenterWidth + (tunerDeltaWidth * i * 2),
							yCenter - tunerHeight, (tunerDeltaWidth - 2) * 2, tunerHeight * 2, TFT_YELLOW);
					}
				}
			}

			serialTFT->setTextColor(0xffff);
			serialTFT->setTextAlignment("mc");
			serialTFT->setTextFont("FSSB24");

			serialTFT->drawText(xCenter, yCenter, noteNames[midiNote % 12]);
			serialTFT->updateScreen();
		}
	}
}

int lastRecordSeconds = -1;

void SerialDisplayInterface::UpdateRecordSeconds(double recordSeconds)
{
	if ((int)recordSeconds == lastRecordSeconds)
		return;

	lastRecordSeconds = (int)recordSeconds;

	int xCenter = screenWidth / 2;
	int yCenter = screenHeight / 2;

	serialTFT->fillRect(0, yCenter - 20, screenWidth, 40, BACKGROUND_COLOR);

	char secondsBuf[6];

	snprintf(secondsBuf, 6, "%02d:%02d", (int)recordSeconds / 60, (int)recordSeconds % 60);

	serialTFT->setTextColor(0xffff);
	serialTFT->setTextAlignment("mc");
	serialTFT->setTextFont("FSSB24");

	serialTFT->drawText(xCenter, yCenter, secondsBuf);

	serialTFT->updateScreen();
}

