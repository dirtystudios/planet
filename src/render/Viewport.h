#pragma once

struct Viewport {
	float topLeftXY[2];
	float width;
	float height;

    bool operator==(const Viewport& other) const {
        return topLeftXY[0] == other.topLeftXY[0] &&
            topLeftXY[1] == other.topLeftXY[1] &&
            width == other.width &&
            height == other.height;
    }
};