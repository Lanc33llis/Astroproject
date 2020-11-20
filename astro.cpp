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
#include <future>
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
#include <chrono>


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
            return Point(*new Point(*poi.get()));
        }
        else {
            throw exception("tried accessing intersection with no intersection!");
        }
    }
    Intersection(double x1, double y1, bool hit) : poi(make_unique<Point>(x1, y1)), hit(hit) {}
    Intersection(Point p, bool hit) : poi(make_unique<Point>(p)), hit(hit) {}
    Intersection(Point p) : poi(make_unique<Point>(p)), hit(true) {}
    Intersection(double x1, double y1) : poi(make_unique<Point>(x1, y1)) {}
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
    Intersection intersect(Ray ray) const {
        auto bs = getBoundingSphere();
        auto x = ray.source.first - bs.p1.first,
             y = ray.source.second - bs.p1.second,
             m = tan(ray.angle * (PI / 180)),
             c1 = cos(ray.angle * (PI / 180));
        auto a = bs.radius;
        auto c = y - m*x;

        auto d = (a * a) + (m * m * a * a) - (c * c);

        if (c1 < 0) { //small fix to angles like 45 equaling the function of 225. 
            //checks if angle is either then reverses or forwards the function and checks distance from object to see if the angle is going away from object
            auto y1 = m * (x - 1) + c;
            if (abs(distance(Point(x - 1, y1), Point(0, 0))) > abs(distance(Point(x, y), Point(0, 0)))) {
                return Intersection(false);
            }
        }
        else {
            auto y1 = m * (x + 1) + c;
            if (abs(distance(Point(x + 1, y1), Point(0, 0))) > abs(distance(Point(x, y), Point(0, 0)))) {
                return Intersection(false);
            }
        }
        

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

                if (c1 <= 0) { //I have no idea what this tries to fix but it works. probably needs to be reimplemented as flawed using cos??
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

class Rectangle : public Shape{
    Point p1, p2;
    BoundingBox bb;
    BoundingSphere bs;
public:
    BoundingBox getBoundingBox() const {
        return bb;
    }
    BoundingSphere getBoundingSphere() const {
        return bs;
    }
    Rectangle(Point p1, Point p2) : p1(p1), p2(p2), bb(BoundingBox(p1, p2)) {}
    Rectangle(double x1, double y1, double x2, double y2) : p1(Point(x1, y1)), p2(Point(x2, y2)) {}
    Intersection intersect (Ray ray) const {
        double hr1 = bb.p1.first, hr2 = bb.p2.first;
        double vr1 = bb.p1.second, vr2 = bb.p2.second;

        auto m = tan(ray.angle * (PI / 180));
        auto c = ray.source.second - m * ray.source.first;

        auto vx1 = m * hr1 + c;
        auto vx2 = m * hr2 + c;
        auto vy1 = (vr1 - c) / m;
        auto vy2 = (vr2 - c) / m;

        double ymin, ymax;
        double xmax, xmin;

        if (min(vr1, vr2) == vr1) {
            ymin = vr1;
            ymax = vr2;
        }
        else {
            ymin = vr2;
            ymax = vr1;
        }

        if (min(hr1, hr2) == hr1) {
            xmin = hr1;
            xmax = hr2;
        }
        else {
            xmin = hr2;
            xmax = hr1;
        }

        auto inRangeOfY = [ymin, ymax](double a) -> bool {
            if (a >= ymin && a <= ymax) {
                return true;
            }
            else {
                return false;
            }
        };

        auto inRangeOfX = [xmin, xmax](double a) -> bool {
            if (a >= xmin && a <= xmax) {
                return true;
            }
            else {
                return false;
            }
        };

        vector<Point> intersections;
        //need to get rid of equal functions of different degrees + ls in rect doesn't look right

        if (inRangeOfY(vx1)) {
            intersections.push_back(Point(hr1, vx1));
        }
        if (inRangeOfY(vx2)) {
            intersections.push_back(Point(hr2, vx2));
        }
        if (inRangeOfX(vy1)) {
            intersections.push_back(Point(vy1, vr1));
        }
        if (inRangeOfX(vy2)) {
            intersections.push_back(Point(vy2, vr2));
        }

        if (intersections.size() == 0) {
            return Intersection(false);
        }
        else if (intersections.size() == 1) {
            return Intersection(intersections[0], true);
        }
        else {
            auto dis1 = distance(intersections[0], ray.source);
            auto dis2 = distance(intersections[1], ray.source);

            if (dis1 < dis2) {
                return Intersection(intersections[0]);
            }
            else {
                return Intersection(intersections[1]);
            }
        }
    }
};

class Canvas : public WPaintedWidget {
public:
    int seed;
    vector<Circle> ps;
    int canvasWidth = 4;
    int canvasHeight = 4;
    int actualWidth;
    int actualHeight;
    double lsX = 0;
    double lsY = 0;
    bool isRun = false;
    int maxPhotonsPerThread = 1500;
    int threads = 4;
    Canvas(int width, int height) : WPaintedWidget(), seed(1), actualHeight(height), actualWidth(width) {
        resize(width, height);
        decorationStyle().setBorder(WBorder(BorderStyle::Solid, BorderWidth::Medium, WColor(0, 0, 0)));
        setStyleClass("canvas");
    }

    void run(int seedr, double x, double y) {
        seed = seedr;
        lsX = x;
        lsY = y;
        isRun = true;
        update();
    }
protected:
    void paintEvent(WPaintDevice* paintDevice) { //good mulithreading, but it isn't covering all of the circle like I would when throwing millions of photons.
        cout << "Paint Event Occured" << "\n";
        WPainter painter;
        painter.begin(paintDevice);
        painter.setBrush(WBrush(WColor(StandardColor::Red)));
        if (isRun) {
            LightSource light(Point(lsX, lsY), 10000000);
            Circle obj1(Point(2, 2), 1);
            //Rectangle obj1(Point(2, 2), Point(3, 3));
            if (light.totalPhotons < maxPhotonsPerThread * threads) {
                int photons = light.totalPhotons / threads;
                vector<future<vector<Circle>>> futures;
                for (size_t i = 0; i < threads; i++) {
                    futures.push_back(
                        async(
                            launch::async,
                            [=]() -> vector<Circle> {
                                vector<Circle> drawn;
                                srand(seed);
                                auto threadNum = i;
                                for (size_t i = 0; i < photons; i++) {
                                    int big = rand() % 360;
                                    int small = rand() % 100;
                                    int smaller = rand() % 100;
                                    int smallest = rand() % 100;
                                    string num = to_string(big) + "." + to_string(small) + to_string(smaller) + to_string(smallest);
                                    auto angle = stod(num);
                                    Ray c(Point(light.p), angle);
                                    auto iData = obj1.intersect(c);
                                    cout << "Ray " << i + 1 << " is going at angle: " << angle << "\n";
                                    if (iData.hit) {
                                        double x = 0, y = 0;
                                        auto poi = iData.getPointOfIntersection();
                                        x = (poi.first / canvasWidth) * actualWidth;
                                        y = (poi.second / canvasHeight) * actualHeight;
                                        cout << "Thread " << threadNum + 1 << " drew at: " << x << " " << y << " going at angle " << angle << "\n";
                                        drawn.push_back(Circle(Point(x, y), 2));
                                    }
                                }
                                return drawn;
                            }
                        )
                    );
                }
                while (true) {
                    chrono::milliseconds now(0);
                    for (size_t i = 0; i < futures.size(); i++) {
                        if (futures[i].wait_for(now) == future_status::ready) {
                            for (Circle a : futures[i].get()) {
                                painter.drawEllipse(a.getBoundingSphere().p1.first, a.getBoundingSphere().p1.second, a.getBoundingSphere().radius, a.getBoundingSphere().radius);
                            }
                            futures.erase(futures.begin() + i);
                            i = 0;
                        }
                    }
                    if (futures.empty()) {
                        break;
                    }
                    else {
                        futures[0].wait_for(chrono::milliseconds(25));
                    }
                }
            }
            else {
                chrono::milliseconds now(0);
                vector<future<vector<Circle>>> futures;
                size_t reallyWorking = 0;
                for (size_t total = 0; total < light.totalPhotons;) {
                    while (futures.size() < threads) {
                        futures.push_back(
                            async(
                                launch::async,
                                [=]() -> vector<Circle> {
                                    vector<Circle> drawn;
                                    srand(seed);
                                    for (size_t i = 0; i < maxPhotonsPerThread; i++) {
                                        int big = rand() % 360;
                                        int small = rand() % 100;
                                        int smaller = rand() % 100;
                                        int smallest = rand() % 100;
                                        string num = to_string(big) + "." + to_string(small) + to_string(smaller) + to_string(smallest);
                                        auto angle = stod(num);
                                        Ray c(Point(light.p), angle);
                                        auto iData = obj1.intersect(c);
                                        //cout << "Ray " << i + 1 << " is going at angle: " << angle << "\n";
                                        if (iData.hit) {
                                            double x = 0, y = 0;
                                            auto poi = iData.getPointOfIntersection();
                                            x = (poi.first / canvasWidth) * actualWidth;
                                            y = (poi.second / canvasHeight) * actualHeight;
                                            //cout << "Thread" << " drew at: " << x << " " << y << " going at angle " << angle << "\n";
                                            drawn.push_back(Circle(Point(x, y), 2));
                                        }
                                    }
                                    return drawn;
                                }
                            )
                        );
                    }
                    for (size_t i = 0; i < futures.size(); i++) {
                        if (futures[i].wait_for(now) == future_status::ready) {
                            for (Circle a : futures[i].get()) {
                                reallyWorking++;
                                painter.drawEllipse(a.getBoundingSphere().p1.first, a.getBoundingSphere().p1.second, a.getBoundingSphere().radius, a.getBoundingSphere().radius);
                            }
                            futures.erase(futures.begin() + i);
                            total += maxPhotonsPerThread;
                        }
                    }
                    if (futures.empty()) {
                        continue;
                    }
                    else {
                        futures[0].wait_for(chrono::milliseconds(1));
                    }
                }
                cout << reallyWorking;
            }
        }
    }
};

void runSim(Canvas *obj) {
    auto canvasWidth = obj->canvasWidth;
    auto canvasHeight = obj->canvasHeight;
    auto actualHeight = obj->actualHeight;
    auto actualWidth = obj->actualWidth;
    auto seed = obj->seed;
    auto lsX = obj->lsX;
    auto lsY = obj->lsY;

    vector<Circle> ps;

    LightSource light(Point(3, 3), 1000); //lsX and lsY don't work!
    ps.push_back(Circle(Point(light.p.first / canvasWidth * actualHeight, light.p.second / canvasHeight * actualHeight), 25));
    Rectangle obj1(Point(2, 2), Point(3, 3));
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
        //root()->addWidget(make_unique<WBreak>());
        auto source = root()->addWidget(make_unique<WLineEdit>("x, y of light source"));
        auto button = root()->addWidget(make_unique<WPushButton>("Start sim!"));
        button->clicked().connect([=] {
            auto a = root()->addWidget(move(sim));
            //auto s = source->text().toUTF8();
            //auto x = stod(&s[1]);
            //auto y = stod(&s[5]);
            a->run(stoi(seed->text()), 0, 0); 
        });
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