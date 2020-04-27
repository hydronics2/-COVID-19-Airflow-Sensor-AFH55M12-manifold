#pragma once

#include <stdint.h>
#include "Color.h"
#include "ILI9341_t3.h"
#include "SensorData.h"
#include "Types.h"

#define GRAPH_X       (30)	// left
#define GRAPH_Y       (25)	// top
#define GRAPH_WIDTH   (290)
#define GRAPH_HEIGHT  (190)

class Display {

public:
	Display();
	void init(int cs, int dc);

	// Redraws labels, and graph axes:
	void setMode(String title, Axis _xAxis, Axis _yAxis);

	// Graphics
	void drawGraph(SensorData * data, uint32_t now, int lastVolumeNumber);

protected:
	ILI9341_t3 * tft;
	uint16_t graph[GRAPH_WIDTH * GRAPH_HEIGHT];
	uint32_t lastDraw = 0;

	// Graph drawing
	void drawEmptyGraph(boolean andWriteToTFT);
	int16_t valueToGraphX(float value);
	int16_t valueToGraphY(float value);
	boolean isPixelInGraph(int16_t x, int16_t y);
	uint16_t getGraphPixel(int16_t x, int16_t y);
	void setGraphPixel(int16_t x, int16_t y, uint16_t color);
	void fillGraph(uint16_t color);
	void drawGraphHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
	void drawGraphVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
	void drawGraphLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

	// Data
	Axis xAxis = {low: -10.0f, high: 0.0f, inc: 2.0f};
	Axis yAxis = {low: 0.0f, high: 20.0f, inc: 2.0f}; 



	// Colors
	uint16_t axisColor = RED;
	uint16_t bgColor = BLACK;	// background color
	uint16_t gridColor = DKBLUE;
	uint16_t plotColor = GREEN;
	uint16_t textColor = WHITE;
};
