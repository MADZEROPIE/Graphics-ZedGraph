#pragma once
#include <vector>
#include <functional>

// Оформление здесь довольно экспериментальное. Заранее извините

typedef std::pair<double, double> dpair;

//ФУНКЦИИ ЗАДАЧ

double f9(double x, double y, double a, double t1) { // Задача 9
    return -a * (y - t1);
}

double f2(double x, double u0, double a, double t) {
    return (u0 - t) * exp(-a * x) + t;
}

struct TResults {
    std::vector<dpair> res_vec;
    std::vector<double> local_mistake_vec;
    std::vector<double> h_vec;
    uint64_t ND = 0;
    uint64_t NH = 0;
    //double max_local_mistake;
};


std::pair<double, double> RK4_new_point(
    std::function<double(double, double,double,double)> f,  // Функция dy/dx
    double x, double y,
    double h,
    double a, double t
) {
    double k1 = f(x, y, a, t);
    double k2 = f(x + (h / 2), y + (h / 2) * k1, a, t);
    double k3 = f(x + (h / 2), y + (h / 2) * k2, a, t);
    double k4 = f(x + h, y + h * k3, a, t);
    x += h; y += h / 6 * (k1 + 2 * k2 + 2 * k3 + k4);
    return std::make_pair(x, y);
}



TResults RungeKutta4 //Метод РГ 4 порядка
(
    std::function<double(double, double, double, double)> f, //Функция du/dx
    double xmin, double xmax, //Начало и конец отрезка интегрирования
    double y0, //Начальные условия
    double h = 0.001, //Шаг интегрирования
    double a = 1.0, double t=0.0,
    bool control = false, //Контроль погрешности
    double eps = 0.000001, //Точность контроля погрешности
    uint64_t NMax = 100 //Максимальное число итераций. Только для версии с переменным шагом.
)
{
    TResults Res;
    std::vector<std::pair<double, double> >& ans=Res.res_vec;
    Res.h_vec.push_back(0.0);
    double x = xmin, y = y0;
    ans.push_back(std::make_pair(x, y));
    unsigned int i = 0;
    Res.local_mistake_vec.push_back(0.0);
    for (; x < xmax-h; ) {  // Если до границы осталось меньше h, то b не используется. FIXED (А надо ли?)
        if (!control) {
            auto tmp = RK4_new_point(f, x, y, h,a,t);
            x = tmp.first; y = tmp.second;
            ans.push_back(tmp);
        }
        else {
            if (i++ >= NMax) return Res;  // Контроль итераций
            auto p1 = RK4_new_point(f, x, y, h,a,t);
            auto p12 = RK4_new_point(f, x, y, h / 2.0,a,t);
            auto p2 = RK4_new_point(f, p12.first, p12.second, h / 2,a,t);
            
            double s = abs(p2.second - p1.second) / 15.0;
            if (s > eps) { h = h / 2.0; ++Res.NH; }
            else {
                x = p1.first; y = p1.second;
                Res.local_mistake_vec.push_back(p2.second - p1.second);
                Res.h_vec.push_back(h);
                if (s < (eps / 32)) {
                    h = h * 2; ++Res.ND;
                }
                ans.push_back(p1);
            }
        }
        
    }
    // Возможно то, что написано ниже является бредом. Относитесь со скептисом.
    if (x + h >= xmax) {
        h = xmax - x;
        if (!control) {
            auto tmp = RK4_new_point(f, x, y, h, a, t);
            x = tmp.first; y = tmp.second;
            ans.push_back(tmp);
        }
        else {
            double s;
            if (i++ > NMax) return Res;
            do {
                auto p1 = RK4_new_point(f, x, y, h, a, t);
                auto p12 = RK4_new_point(f, x, y, h / 2.0, a, t);
                auto p2 = RK4_new_point(f, p12.first, p12.second, h / 2.0, a, t);
                
                s = abs(p2.second - p1.second) / (15.0);
                if (s > eps){ h = h / 2; ++Res.NH;}
                else {
                    Res.local_mistake_vec.push_back(p2.second - p1.second);
                    Res.h_vec.push_back(h);
                    x = p1.first; y = p1.second;
                    ans.push_back(p1);
                }
            } while (s > eps);
    
        }
    }

    return Res;
}
