#include <iostream>
#include <chrono>
#include <cmath>
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


using namespace Wt;
using namespace std;

long operator"" px(unsigned long long a) {
    return a;
}

constexpr auto PI = 3.14159265358979323846;

typedef pair<double, double> Point;
typedef pair<double, double> MathVector;

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
};

struct BoundingSphere {
    Point p1;
    double radius;
    BoundingSphere(double x1, double y1, double radius) : p1(Point(x1, y1)), radius(radius) {}
    BoundingSphere(Point p1, double radius) : p1(p1), radius(radius) {}
};

struct Intersection {
    unique_ptr<Point> getPointOfIntersection() {
        if (hit) {
            return make_unique<Point>(poi.get());
        }
        else {
            return false;
        }
    }
    Intersection(double x1, double y1, bool hit) : poi(make_unique<Point>(x1, y1)), hit(hit) {}
    Intersection(Point p, bool hit) : poi(make_unique<Point>(p)), hit(hit) {}
private:
    unique_ptr<Point> poi;
    bool hit;
};

class LightSource {
    size_t totalPhotons;
    vector<unsigned char> color;
    LightSource(size_t numPhotons, vector<unsigned char> RGB = {255, 255, 255, 1}) : totalPhotons(numPhotons), color(RGB) {}
};

class Shape {
public:
    virtual BoundingBox getBoundingBox() = 0;
    virtual BoundingSphere getBoundingShere() = 0;
    Intersection Intersect(Ray r) {
        auto x1 = r.source.first;
        auto y1 = r.source.second;
        double x2, y2;
        if (r.angle > 0) {
            x2 = 4;
        }
        else {
            x2 = -4;
        }
        y2 = tan(r.angle * (PI / 180)) * x2;
        auto rad = getBoundingShere().radius;

        auto dx = x2 - x1;
        auto dy = y2 - y1;
        auto dr = sqrt((dx * dx) + (dy * dy));
        auto D = (x1 * y2) - (x2 * y1);

        int sgn;
        if (dy < 0) {
            sgn = -1;
        } 
        else
        {
            sgn = 1;
        }

        auto px1 = ((D * dy) + (sgn * dx * sqrt((rad * rad * dr * dr) - (D * D)))) / (dr * dr);
        auto px2 = ((D * dy) - (sgn * dx * sqrt((rad * rad * dr * dr) - (D * D)))) / (dr * dr);
        auto py1 = ((-D * dx) + (abs(dy) * sqrt((rad * rad * dr * dr) - (D * D)))) / (dr * dr);
        auto py1 = ((-D * dx) - (abs(dy) * sqrt((rad * rad * dr * dr) - (D * D)))) / (dr * dr);

        
    }
    Shape() {}
};

class Canvas : public WPaintedWidget {
public:
    Canvas(int width, int height) : WPaintedWidget() {
        resize(width, height);
        decorationStyle().setBorder(WBorder(BorderStyle::Solid, BorderWidth::Medium, WColor(0, 0, 0)));
        mouseWentDown().connect([this]() { cout << "this is working if you press down in this arae!" << "\n"; });
        setStyleClass("canvas");
    }

protected:
    void paintEvent(WPaintDevice* paintDevice) {
        cout << "overriding virtual function" << "\n";
    }

private:
    void run() {
        while (true) {

        }
    }
};

class astroProject : public WApplication
{
    unique_ptr<Canvas> sim;
public:
    astroProject(const WEnvironment& env) : WApplication(env), sim(make_unique<Canvas>(1600, 800)) {
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
        else if (path == "/test") {
            testPage();
        }
    }

    void home() {
        setTitle("Photometry?");
        header();
        root()->addWidget(move(sim));
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