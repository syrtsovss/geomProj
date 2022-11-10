#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

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
    sf::Vector2i pointA, pointB;
    //Точка на стороне
    sf::Vector2i pointP;
    //Флаг, рассчитан ли прямоугольник
    bool isSolved=false;
    //Оставшиеся вершины
    sf::Vector2f pointC={0, 0}, pointD={0, 0};
    Rect(const sf::Vector2i &pointA, const sf::Vector2i &pointB, const sf::Vector2i &pointP) : 
        pointA(pointA), pointB(pointB), pointP(pointP){}
};

//Вектор прямоугольников
std::vector<Rect> rectangles;

//Координаты следующего прямоугольника
int nextRect[3][2]={{0, 0}, {0, 0}, {0, 0}};
//Номер клика мыши
char counter=0;
//Количество созданных случайных прямоугольников
int nextRandom=10;
//Пути к файлу для чтения и записи
std::string pathIn="/home/m/in.txt";
std::string pathOut="/home/m/out.txt";

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

//Расчёт вершин прямоугольника
void solveRect(Rect * rect){
    //Вектор AD
    sf::Vector2f AD;
    /*Если точки A и B совпадают, 
    то прямоугольник не определён корректно
    Будем считать, что он совпадает с отрезком AP*/
    if (rect->pointA==rect->pointB){
        AD=static_cast<sf::Vector2f>(rect->pointP-rect->pointA);
    }
    //Если A и B не совпадают
    else{
        //Вычисляем вектор вдоль стороны и от вершины до точки на стороне
        sf::Vector2f AB=static_cast<sf::Vector2f>(rect->pointB-rect->pointA);
        sf::Vector2f AP=static_cast<sf::Vector2f>(rect->pointP-rect->pointA);
        //Проецируем AP на прямую, перпендикулярную AB
        sf::Vector2f orthogonal=norm(turn(AB));
        float lenthAD=orthogonal*AP;
        //Вычисляем AD
        AD=lenthAD*orthogonal;
    }
    //Вычисляем оставшиеся вершины прямоугольника
    rect->pointD=static_cast<sf::Vector2f>(rect->pointA)+AD;
    rect->pointC=static_cast<sf::Vector2f>(rect->pointB)+AD;
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
        if(!rect.isSolved){
            solveRect(&rect);
        }
        sf::Vector2i pointD=static_cast<sf::Vector2i>(rect.pointD);
        sf::Vector2i pointC=static_cast<sf::Vector2i>(rect.pointC);

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

//Добавление прямоугольника
void addRect(){
    rectangles.push_back(Rect(
        sf::Vector2i(nextRect[0][0], nextRect[0][1]),
        sf::Vector2i(nextRect[1][0],nextRect[1][1]),
        sf::Vector2i(nextRect[2][0],nextRect[2][1])));
}

//Отрисовка инструмента добавления прямоугольника
void showRect(){
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
        addRect();
    }
}
//Добавление случайных прямоугольников
void addRandom(int num, sf::RenderWindow * windowPtr){
    //Получаем текущий размер окна
    sf::Vector2u size=(*windowPtr).getSize();
    //Добавляем в цикле прямоугольники
    for(int i=0; i<num; i++){
        //Получаем случайные координаты точек
        sf::Vector2i pointA=sf::Vector2i(rand()%size.x, rand()%size.y);
        sf::Vector2i pointB=sf::Vector2i(rand()%size.x, rand()%size.y);
        sf::Vector2i pointP=sf::Vector2i(rand()%size.x, rand()%size.y);
        //Если точки на стороне совпали
        if(pointA==pointB)
            //Смещаем одну из точек
            pointB=pointB+sf::Vector2i(2*(rand()%2)-1,2*(rand()%2)-1);
        //Добавляем прямоугольник
        rectangles.push_back(Rect(pointA, pointB, pointP));
    }
}

//Отрисовка инструмента добавления случайных прямоугольников
void showRandom(sf::RenderWindow * windowPtr){
    //Проверяем, раскрыта ли панель "Add random rectangles"
    if(!ImGui::CollapsingHeader("Add random rectangles"))
        return;
    //Инструмент выбора числа случайных прямоугольников
    if(ImGui::DragInt("Number of rectangles", &nextRandom)){
        //Если количество прямоугольников отрицательно, обнуляем
        if(nextRandom<0) nextRandom=0;
    }
    //Если нажата кнопка "Add", добавляем прямоугольники
    if(ImGui::Button("Add")){
        addRandom(nextRandom, windowPtr);
    }
}
//Запись в файл
void writeToFile(){
    //Открываем файл
    std::ofstream file;
    file.open(pathOut);
    //Если файл не открывается
    if(!file){
        std::cerr << "Can't open file";
        return;
    }
    //Для каждого прямоугольника
    for(auto rect: rectangles){
        //Записываем в файл координаты точек прямоугольника
        file << rect.pointA.x << ' ' << rect.pointA.y << ' ';
        file << rect.pointB.x << ' ' << rect.pointB.y << ' ';
        file << rect.pointP.x << ' ' << rect.pointP.y << '\n';
    }
    //Закрываем файл
    file.close();
}

//Чтение из файла
void readFromFile(){
    //Открываем файл
    std::ifstream file;
    file.open(pathIn);
    //Если файл не открывается
    if(!file){
        std::cerr << "Can't open file";
        return;
    }
    //Очищаем вектор прямоугольников
    rectangles.clear();
    //Переменные для чтения из файла
    int pointAX,pointAY,pointBX,pointBY,pointPX,pointPY;
    //Пока не конец файла
    while(file >> pointAX >> pointAY >> 
        pointBX >> pointBY >> 
        pointPX >> pointPY){
        //Создаём прямоугольник
        rectangles.push_back(Rect(
        sf::Vector2i(pointAX, pointAY),
        sf::Vector2i(pointBX, pointBY),
        sf::Vector2i(pointPX, pointPY)));
    }
    file.close();
}

void showFile(){
    if(!ImGui::CollapsingHeader("Files"))
        return;
    if(ImGui::Button("Load"))
        readFromFile();
    ImGui::SameLine();
    if(ImGui::Button("Save"))
        writeToFile();
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
            //Если событие - клик мыши, и оно не обработано элементами ImGui
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button==sf::Mouse::Button::Left && !ImGui::GetIO().WantCaptureMouse){
                //Получаем координаты
                nextRect[counter][0]=event.mouseButton.x;
                nextRect[counter][1]=event.mouseButton.y;
                //Прибавляем 1 к счётчику
                counter++;
                //Если прошло три нажатия с последнего добавления прямоугольника мышью
                if(counter==3){
                    //Добавляем прямоугольник
                    addRect();
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
        showRect();
        // Добавление случайных прямоугольников
        showRandom(&window);
        //Работа с файлами
        showFile();
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