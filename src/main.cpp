#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <cmath>
#include <vector>

const int SIZE_X=1080;
const int SIZE_Y=720;

// Начальный цвет фона в RGBA
float color[3]={0, 0.007, 0.223};
// Цвет фона
static sf::Color bg;
// Задаём цвет фона
void setColor(float * newColor){
    bg.r=static_cast<unsigned char> (newColor[0]*255.f);
    bg.g=static_cast<unsigned char> (newColor[1]*255.f);
    bg.b=static_cast<unsigned char> (newColor[2]*255.f);
}

// Прямоугольник
struct Rect{
    //Вершины
    sf::Vector2i pointA;
    sf::Vector2i pointB;
    //Точка на стороне
    sf::Vector2i pointP;
    Rect(const sf::Vector2i &pointA, const sf::Vector2i &pointB, const sf::Vector2i &pointP) : 
        pointA(pointA), pointB(pointB), pointP(pointP){}
};

//Вектор прямоугольников
std::vector<Rect> rectangles;

//Координаты следующего прямоугольника
int nextRect[3][2]={{0, 0}, {0, 0}, {0, 0}};
//Номер клика мыши
char counter=0;

//Операции с векторами

//Скалярное произведение
float operator*(sf::Vector2f a, sf::Vector2f b){
    return a.x*b.x+a.y*b.y;
}
//Нормирование
sf::Vector2f norm(sf::Vector2f a){
    //Модуль вектора
    float abs=sqrt(a.x*a.x+a.y*a.y); 
    return sf::Vector2f(a.x/abs, a.y/abs);
}
//Поворот на прямой угол против часовой стрелки
sf::Vector2f turn(sf::Vector2f a){
    return sf::Vector2f(-a.y, a.x);
}

//Рисуем задачу
void render(sf::Vector2u size){
    // Задаём размеры и положение невидимого окна
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(ImVec2(size.x,size.y));
    // Создаём окно
    ImGui::Begin("Overlay", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground);
    //Получаем список примитивов
    auto drawList=ImGui::GetWindowDrawList();
    for (auto rect: rectangles){
        //Вычисляем вектор вдоль стороны и от вершины до точки на стороне
        sf::Vector2f AB=static_cast<sf::Vector2f>(rect.pointB-rect.pointA);
        sf::Vector2f AP=static_cast<sf::Vector2f>(rect.pointP-rect.pointA);
        //Проецируем AP на прямую, перпендикулярную AB
        sf::Vector2f orthogonal=norm(turn(AB));
        float lenthAD=orthogonal*AP;
        //Вычисляем AD
        sf::Vector2f AD=lenthAD*orthogonal;
        //Вычисляем оставшиеся вершины треугольника
        sf::Vector2i pointD=rect.pointA+static_cast<sf::Vector2i>(AD);
        sf::Vector2i pointC=rect.pointB+static_cast<sf::Vector2i>(AD);

        //Рисуем стороны прямоугольника
        drawList->AddLine(
            rect.pointA, rect.pointB,
            ImColor(100, 150, 200), 
            1.5
        );
        drawList->AddLine(
            rect.pointB, pointC,
            ImColor(100, 150, 200), 
            1.5
        );
        drawList->AddLine(
            pointC, pointD,
            ImColor(100, 150, 200), 
            1.5
        );
        drawList->AddLine(
            pointD, rect.pointA,
            ImColor(100, 150, 200), 
            1.5
        );
    }
    for (int i=0; i<counter; i++){
        drawList->AddCircleFilled(
            sf::Vector2i(nextRect[i][0],nextRect[i][1]),
            3,
            ImColor(200, 100, 150)
        );
    }
    ImGui::End();

}

//Отрисовка инструмента изменения цвета фона
void backgroundSettings(){
    //Проверяем, раскрыта ли панель "Background color"
    if(!ImGui::CollapsingHeader("Background color"))
        return;
    //Выбор цвета
    if (ImGui::ColorEdit3("Background color", color)) {
        // Если значение color изменилось, меняем цвет фона
        setColor(color);
    }
}
//Отрисовка инструмента добавления прямоугольника
void addRect(){
    //Проверяем, раскрыта ли панель "Add rectangle"
    if(!ImGui::CollapsingHeader("Add rectangle"))
        return;
    //Выбор координат точек 
    if(ImGui::DragInt2("Point A", nextRect[0]));
    if(ImGui::DragInt2("Point B", nextRect[1]));
    if(ImGui::DragInt2("Point on line CD", nextRect[2]));
    //Если нажата кнопка "Add"
    if(ImGui::Button("Add")){
        //Добавляем прямоугольник
        rectangles.push_back(Rect(
            sf::Vector2i(nextRect[0][0], nextRect[0][1]),
            sf::Vector2i(nextRect[1][0],nextRect[1][1]),
            sf::Vector2i(nextRect[2][0],nextRect[2][1])));
    }
}

int main() {
    // Создаём окно
    sf::RenderWindow window(sf::VideoMode(SIZE_X, SIZE_Y), "Geometry project");
    
    // Задаём частоту перерисовки окна
    window.setFramerateLimit(60);
    // Инициализация ImGUI
    ImGui::SFML::Init(window);
    // Задаём цвет фона
    setColor(color);
    // Переменная таймера
    sf::Clock timer;
    while (window.isOpen()) {
        // Создаём событие
        sf::Event event;
        // Отправляем событие на обработку sfml
            ImGui::SFML::ProcessEvent(event);
        // Пока окно принимает события
        while (window.pollEvent(event)) {
            // Если событие - закрытие окна, закрываем окно
            if (event.type == sf::Event::Closed)
                window.close();
            // Если нажато Ctrl+Q, закрываем окно
            if (event.type == sf::Event::KeyPressed && event.key.code==sf::Keyboard::Q && event.key.control)
                window.close();
            //Если событие - клик мыши
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button==sf::Mouse::Button::Left){
                //Получаем координаты
                nextRect[counter][0]=event.mouseButton.x;
                nextRect[counter][1]=event.mouseButton.y;
                //Прибавляем 1 к счётчику
                counter++;
                //Если прошло три нажатия с последнего добавления прямоугольника мышью
                if(counter==3){
                    //Добавляем прямоугольник
                    rectangles.push_back(Rect(
                        sf::Vector2i(nextRect[0][0], nextRect[0][1]),
                        sf::Vector2i(nextRect[1][0],nextRect[1][1]),
                        sf::Vector2i(nextRect[2][0],nextRect[2][1])));
                    //Обнуляем счётчик
                    counter=0;
                }
            }
        }
        // Запускаем обновление окна по таймеру с заданной частотой
        ImGui::SFML::Update(window, timer.restart());
        //Рисуем задачу
        render(window.getSize());
        // Создаём окно управления
        ImGui::Begin("Control");
        // Выбор цвета
        backgroundSettings();
        // Добавление прямоугольника
        addRect();
        // Заканчиваем рисовать окно
        ImGui::End();
        // Очищаем окно
        window.clear(bg);
        // Рисуем средствами Imgui+sfml
        ImGui::SFML::Render(window);
        // Отображаем изменения на окне
        window.display();
    }

    return 0;
}