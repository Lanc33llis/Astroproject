#include <iostream>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <thread>
#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WCssDecorationStyle.h>
#include <Wt/WColor.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEvent.h>
#include <Wt/WPainter.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainterPath.h>
#include <Wt/WPen.h>
#include <Wt/WPointF.h>
#include <Wt/WPushButton.h>
#include <Wt/WRectF.h>
#include <Wt/WTemplate.h>
#include <Wt/WToolBar.h>
#include <Wt/WSvgImage.h>
#include <Wt/WPaintDevice.h>
#include <Wt/WSpinBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPaintDevice.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainter.h>
#include <Wt/WRasterImage.h>
#include <Wt/WCanvasPaintDevice.h>





using namespace Wt;
using namespace std;

long operator"" px(unsigned long long a) {
    return a;
}

constexpr auto PI = 3.14159265358979323846;

typedef pair<double, double> Point;

double distance(Point p1, Point p2) {
    return sqrt(pow(p2.first - p1.first, 2) + pow(p2.second - p2.second, 2));
}   

struct Ray {
    Point source;
    double angle;
    Ray(double x1, double y1, double angle) : source(Point(x1, y1)), angle(angle) {}
    Ray(Point s, double angle) : Ray(s.first, s.second, angle) {}
};

struct BoundingBox {
    Point p1, p2;
    BoundingBox(double x1, double y1, double x2, double y2) : p1(Point(x1, y1)), p2(Point(x2, y2)) {}
    BoundingBox(Point p1, Point p2) : p1(p1), p2(p2) {}
    BoundingBox() {}
};

struct BoundingSphere {
    Point p1;
    double radius;
    BoundingSphere() {}
    BoundingSphere(double x1, double y1, double radius) : p1(Point(x1, y1)), radius(radius) {}
    BoundingSphere(Point p1, double radius) : p1(p1), radius(radius) {}
    bool pointExists(Point p) {
        auto dis = distance(p, p1);
        if (dis <= radius) {
            return true;
        }
        else {
            return false;
        }
    }
};

struct Intersection {
    bool hit;
    Point getPointOfIntersection() {
        if (hit) {
            return Point(poi->first, poi->second);
            //return make_unique<Point>(0, 0);
        }
    }
    Intersection(double x1, double y1, bool hit) : poi(make_unique<Point>(x1, y1)), hit(hit) {}
    Intersection(Point p, bool hit) : poi(make_unique<Point>(p)), hit(hit) {}
    Intersection(bool hit) : poi(nullptr), hit(hit) {}
private:
    unique_ptr<Point> poi;
};

class LightSource {
public:
    LightSource(Point p, size_t numPhotons, vector<unsigned char> RGB = {255, 255, 255, 1}) : p(p), totalPhotons(numPhotons), color(RGB) {}
    size_t totalPhotons;
    vector<unsigned char> color;
    Point p;
};

class Shape {
public:
    Shape() {}
    virtual BoundingBox getBoundingBox() const = 0;
    virtual BoundingSphere getBoundingSphere() const = 0;
    Intersection intersect(Ray ray) {
        auto bs = getBoundingSphere();
        auto x = ray.source.first - bs.p1.first,
             y = ray.source.second - bs.p1.second,
             m = tan(ray.angle * (PI / 180)),
             c1 = cos(ray.angle * (PI / 180));
        auto a = bs.radius;
        auto c = y - m*x;

        auto d = (a * a) + (m * m * a * a) - (c * c);

        auto m1 = y / x;
        auto c1 = y - m1 * x;
        auto a1 = atan(m1) * 180 / PI;
        //need to find how much circle takes up field of view for light source. use that and filter out other angles.

        if (d > 0) { //this ignores tangents but gives certainy of two points
            auto x1 = (((-m * c) + sqrt(d)) / (1 + m * m));
            auto x2 = (-(((m * c) + sqrt(d)) / (1 + m * m)));

            auto y1 = m * x1 + c;
            auto y2 = m * x2 + c;

            Point p1(x1 + bs.p1.first, y1 + bs.p1.second);
            Point p2(x2 + bs.p1.first, y2 + bs.p1.second);

            auto dis1 = distance(ray.source, p1);
            auto dis2 = distance(ray.source, p2);

            if (distance(Point(0, 0), Point(x, y)) <= a) {

                if (c1 >= 0) {
                    if (p1.first > p2.first) {
                        return Intersection(p1, true);
                    }
                    else {
                        return Intersection(p2, true);
                    }
                }
                else {
                    if (p1.first > p2.first) {
                        return Intersection(p2, true);
                    }
                    else {
                        return Intersection(p1, true);
                    }
                }

            }

            if (dis1 - dis2 > 0) {
                return Intersection(p2, true);
            }
            else {
                return Intersection(p1, true);
            }
        }
        else {
            return Intersection(Point(0, 0), false);
        }
    }
};

bool operator ==(const Shape &s1, const Shape &s2) {
    auto bs1 = s1.getBoundingSphere();
    auto bs2 = s2.getBoundingSphere();
    if (bs1.p1 == bs2.p1 && bs2.radius == bs1.radius) {
        return true;
    }
    else {
        return false;
    }
}

bool operator !=(const Shape &s1, const Shape &s2) {
    if (s1 == s2) {
        return false;
    }
    else {
        return true;
    }
}

class Circle : public Shape {
    BoundingBox bb;
    BoundingSphere bs;
    double radius;

public:

    Circle() {}
    Circle(Point center, double radius) : bs(BoundingSphere(center, radius)) {
        auto p1 = Point(center.first - radius, center.second + radius);
        auto p2 = Point(center.first + radius, center.second - radius);
        bb = BoundingBox(p1, p2);
    }
    BoundingBox getBoundingBox() const {
        return bb;
    }
    BoundingSphere getBoundingSphere() const {
        return bs;
    }
    double getRadius() {
        return radius;
    }
    double setRadius(double in) {
        radius = in;
    }
};

//class Rectangle {
//    Point p1, p2;
//    BoundingBox bb;
//    BoundingSphere bs;
//    BoundingBox getBoundingBox() {
//        return bb;
//    }
//    BoundingSphere getBoundingSphere() {
//        return bs;
//    }
//    Rectangle(Point p1, Point p2) : p1(p1), p2(p2) {}
//    Rectangle(double x1, double y1, double x2, double y2) : p1(Point(x1, y1)), p2(Point(x2, y2)) {}
//    Intersection intersect(Ray ray) {
//        double hr1 = bb.p1.first, hr2 = bb.p2.first;
//        double vr1 = bb.p1.second, vr2 = bb.p2.second;
//        
//        auto m = tan(ray.angle * (PI / 180));
//        auto c = ray.source.second - m * ray.source.first;
//
//        auto vx1 = m * hr1 + c;
//        auto vx2 = m * hr2 + c;
//        auto vy1 = (vr1 - c) / m;
//        auto vy1 = (vr2 - c) / m;
//
//        double ymin, ymax;
//
//        if (cos(ray.angle) > 0 && hr1) {
//            if (min(vr1, vr2) ==  vr1) {
//                ymin = vr1;
//                ymax = vr2;
//            }
//            else {
//                ymin = vr2;
//                ymax = vr1;
//            }
//
//            if (vx1 >= ymin && vx1 <= ymax) {
//
//            }
//        }
//
//    }
//};

class Canvas : public WPaintedWidget {
public:
    int seed;
    vector<Circle> ps;
    int canvasWidth = 4;
    int canvasHeight = 4;
    int actualWidth;
    int actualHeight;
    Canvas(int width, int height) : WPaintedWidget(), seed(1), actualHeight(height), actualWidth(width) {
        resize(width, height);
        decorationStyle().setBorder(WBorder(BorderStyle::Solid, BorderWidth::Medium, WColor(0, 0, 0)));
        setStyleClass("canvas");
    }

    void run(int seedr) {
        seed = seedr;
    }
protected:
    void paintEvent(WPaintDevice* paintDevice) {
        cout << "Paint Event Occured" << "\n";
        WPainter painter;
        painter.begin(paintDevice);
        painter.setBrush(WBrush(WColor(StandardColor::Red)));
        for (Circle a : ps) {
            painter.drawEllipse(a.getBoundingSphere().p1.first, a.getBoundingSphere().p1.second, a.getBoundingSphere().radius, a.getBoundingSphere().radius);
        }
    }
};

void runSim(Canvas *obj) {
    auto canvasWidth = obj->canvasWidth;
    auto canvasHeight = obj->canvasHeight;
    auto actualHeight = obj->actualHeight;
    auto actualWidth = obj->actualWidth;
    auto seed = obj->seed;

    vector<Circle> ps;

    LightSource light(Point(0, 4), 1000);
    ps.push_back(Circle(Point(light.p.first / canvasWidth * actualHeight, light.p.second / canvasHeight * actualHeight), 25));
    Circle obj1(Point(2, 2), 1);
    srand(seed);
    for (size_t i = 0; i < light.totalPhotons; i++) {
        int big = rand() % 360;
        int small = rand() % 100;
        int smaller = rand() % 100;
        string num = to_string(big) + "." + to_string(small) + to_string(smaller);
        auto angle = stod(num);
        cout << "Ray " << i + 1 << " is going at angle: " << angle << "\n";
        Ray c(Point(light.p), angle);
        auto iData = obj1.intersect(c);
        if (iData.hit) {
            double x = 0, y = 0;
            auto poi = iData.getPointOfIntersection();
            x = (poi.first / canvasWidth) * actualWidth;
            y = (poi.second / canvasHeight) * actualHeight;
            ps.push_back(Circle(Point(x, y), 5));
            cout << "drew at: " << x << " " << y << "\n";
        }
    }
    obj->ps = ps;
    obj->update();
}

class astroProject : public WApplication
{
    unique_ptr<Canvas> sim;
public:
    astroProject(const WEnvironment& env) : WApplication(env), sim(make_unique<Canvas>(700, 700)) {
        internalPathChanged().connect([this] {handleInternalPath(); });
        useStyleSheet("style.css");
        handleInternalPath();
    }

    void handleInternalPath() {
        root()->clear();
        std::string path = this->internalPath();
        if (path == "/") {
            home();
        }
    }

    void home() {
        setTitle("Photometry?");
        header();
        auto seed = root()->addWidget(make_unique<WLineEdit>("seed"));
        auto button = root()->addWidget(make_unique<WPushButton>("Start sim!"));
        button->clicked().connect([=] {auto a = root()->addWidget(move(sim)); a->run(stoi(seed->text())); thread e(runSim, a); e.join(); });
    }

    void header() {
        auto container = make_unique<WContainerWidget>();   
        container->setStyleClass("header");
        auto title = make_unique<WText>("Quick little photometry");
        container->addWidget(unique_ptr<WWidget>(move(title)));
        root()->addWidget(unique_ptr<WWidget>(move(container)));
    }

    void testPage() {
        setTitle("Testing Path Change???");
        auto test = root()->addWidget(std::make_unique<WText>("Howdy!"));
    }

};

int main(int argc, char** argv){
    return WRun(argc, argv, [](const WEnvironment& env) {
        return std::make_unique<astroProject>(env);
    });
 }