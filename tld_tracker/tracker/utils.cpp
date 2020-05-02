#include <tracker/utils.h>

std::ostream& operator<<(std::ostream &os, const cv::Rect& rect) {
    os << "Rect: " << rect.x << " " << rect.y << " "
       << rect.width << " " << rect.height << std::endl;
    return os;
}
