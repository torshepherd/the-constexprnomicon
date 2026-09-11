#include "references.cpp"

int main(int argc, char**) {
    coordinate point{argc, 20};
    return !(before(point) && after(point)
             && point.x == argc + 3 && point.y == 23);
}
