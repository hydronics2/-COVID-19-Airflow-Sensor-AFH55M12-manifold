#include <stdint.h>
#include <Arduino.h>
#include "Display.h"
#include "ILI9341_t3.h"
#include "Types.h"

static_assert(0 <= GRAPH_X, "Invalid GRAPH_X, graph will appear corrupted");
static_assert((GRAPH_X + GRAPH_WIDTH) <= 320, "Invalid graph width, graph will appear corrupted");
static_assert(0 <= GRAPH_Y, "Invalid GRAPH_Y, graph will appear corrupted");
static_assert((GRAPH_Y + GRAPH_HEIGHT) <= 240, "Invalid graph height, graph will appear corrupted");

// Convert uint8_t r,g,b to TFT color uint16
#define color565(r,g,b)   (((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)

Display::Display() {}

void Display::init(int cs, int dc)
{
	tft = new ILI9341_t3(cs, dc);
	tft->begin();
	tft->fillScreen(bgColor);
	tft->setTextColor(textColor);
	tft->setTextSize(2);
	tft->setRotation(1);
	//tft.println("Waiting for Arduino Serial Monitor...");
}

void Display::setMode(String title, Axis _xAxis, Axis _yAxis)
{
	xAxis = _xAxis;
	yAxis = _yAxis;


	// Convenient shorthand variables
	const int16_t gx = GRAPH_X;
	const int16_t gy = GRAPH_Y;
	const int16_t gh = GRAPH_HEIGHT;

	// Title
	tft->fillRect(gx, 0, tft->width(), GRAPH_Y, bgColor);	// Erase old title

	tft->setTextColor(textColor, bgColor);
	tft->setTextSize(2);
	tft->setCursor(gx, gy - 30);
	tft->println(title);

  // Axis Values: X
	tft->fillRect(0, GRAPH_Y + GRAPH_HEIGHT + 1, tft->width(), tft->height() - (GRAPH_Y + GRAPH_HEIGHT), bgColor);	// Erase old X values

	tft->setTextSize(1);
	for (float f = xAxis.low; f < xAxis.high; f += xAxis.inc) {
		int16_t drawX = GRAPH_X + valueToGraphX(f);

		tft->setCursor(drawX, GRAPH_Y + GRAPH_HEIGHT + 2);
		tft->println((int)f);
	}

	// Axis values: Y
	tft->fillRect(0, 0, GRAPH_X - 1, tft->height(), bgColor);	// Erase old Y values

	for (float f = yAxis.low; f <= yAxis.high; f += yAxis.inc) {
		int16_t drawY = GRAPH_Y + valueToGraphY(f);

		const int16_t nudgeTextBaseline = 4;
		tft->setCursor(0, drawY - nudgeTextBaseline);
		tft->println((int)f);
	}

	// Graph left border
	tft->drawFastVLine(gx - 1, gy, gh, axisColor);

	// Immediately clear the graph
	drawEmptyGraph(true);
}

// -- GRAPHICS --

void Display::drawGraph(SensorData * data, uint32_t now, int lastMaxVolume)
{
	// Avoid shimmering pixels (rounding errors):
	// Only redraw when +1px scrolling can happen.
	//int32_t sinceDraw = (int32_t)(now - lastDraw);
	int32_t pxInterval = ((xAxis.high - xAxis.low) * 1000.0f) / GRAPH_WIDTH;

	int32_t sinceDraw = (int32_t)(now - lastDraw);
	if (sinceDraw < pxInterval) {
		return;
	}

	// Round down to start of pixel, to avoid shimmering lines.
	now -= (now % pxInterval);
	lastDraw = now;

	drawEmptyGraph(false);

	// Plot data values
	int16_t lastX, lastY;
	int32_t len = data->length();
	for (int32_t i = 0; i < len; i++) {
		float seconds = (int32_t)(data->at(i)->millis - now) / 1000.0f;
		int16_t drawX = valueToGraphX(seconds);
		int16_t drawY = valueToGraphY(data->at(i)->value);

		if (i > 0) {
			drawGraphLine(lastX, lastY, drawX, drawY, plotColor);
		}

		lastX = drawX;
		lastY = drawY;
	}

	tft->writeRect(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT, graph);  

  //Serial.println(lastMaxVolume);
  tft->setTextSize(2);
  tft->setCursor(245, 5);
  tft->print(lastMaxVolume);
  tft->println("mL");

}

// -- PROTECTED --

void Display::drawEmptyGraph(boolean andWriteToTFT)
{
	fillGraph(bgColor);

	// Draw X markers (vertical rules)
	for (float f = xAxis.low; f <= xAxis.high; f += xAxis.inc) {
		int16_t drawX = valueToGraphX(f);
		drawGraphVLine(drawX, 0, GRAPH_HEIGHT, gridColor);
	}

	// Draw Y markers (horizontal rules)
	for (float f = yAxis.low; f <= yAxis.high; f += yAxis.inc) {
		int16_t drawY = valueToGraphY(f);
		drawGraphHLine(0, drawY, GRAPH_WIDTH, gridColor);
	}

	// Draw X axis inside graph area:
	drawGraphHLine(0, valueToGraphY(0), GRAPH_WIDTH, axisColor);

	if (andWriteToTFT) {
		tft->writeRect(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT, graph);
	}
}

int16_t Display::valueToGraphX(float value)
{
	// Graph bitmap coordinates begin at (0,0)
	const int16_t graphLeft = 0;
	const int16_t graphRight = GRAPH_WIDTH - 1;

	return map(value, xAxis.low, xAxis.high, graphLeft, graphRight);
}

int16_t Display::valueToGraphY(float value)
{
	// Graph bitmap coordinates begin at (0,0)
	const int16_t graphTop = 0;
	const int16_t graphBottom = GRAPH_HEIGHT - 1;

	return map(value, yAxis.low, yAxis.high, graphBottom, graphTop);
}

boolean Display::isPixelInGraph(int16_t x, int16_t y)
{
	if ((x < 0) || (GRAPH_WIDTH <= x)) return false;
	if ((y < 0) || (GRAPH_HEIGHT <= y)) return false;
	return true;
}

uint16_t Display::getGraphPixel(int16_t x, int16_t y)
{
	if (!isPixelInGraph(x, y)) return 0x0;

	return graph[y * GRAPH_WIDTH + x];
}

void Display::setGraphPixel(int16_t x, int16_t y, uint16_t color)
{
	if (!isPixelInGraph(x, y)) return;

	graph[y * GRAPH_WIDTH + x] = color;
}

void Display::fillGraph(uint16_t color)
{
	for (int32_t i = 0; i < (GRAPH_WIDTH * GRAPH_HEIGHT); i++) {
		graph[i] = color;
	}
}

void Display::drawGraphHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	// Negative width? Prepare for drawing left-to-right:
	if (w < 0) {
		x += w;
		w = -w;
	}

	for (int16_t i = 0; i < w; i++) {
		setGraphPixel(x + i, y, color);
	}
}

void Display::drawGraphVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
	// Negative height? Prepare for drawing top-to-bottom:
	if (h < 0) {
		y += h;
		h = -h;
	}

	for (int16_t j = 0; j < h; j++) {
		setGraphPixel(x, y + j, color);
	}
}

void Display::drawGraphLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
	int16_t xLen = abs(x1 - x2);
	int16_t yLen = abs(y1 - y2);

	// Stepping horizontally, or vertically?
	int16_t steps = max(xLen, yLen);

	float dx = (float)(x2 - x1) / steps;
	float dy = (float)(y2 - y1) / steps;

	float x = x1, y = y1;	// Start coordinates
	for (int16_t i = 0; i <= steps; i++) {
		setGraphPixel(x, y, color);
		x += dx;
		y += dy;
	}
}
