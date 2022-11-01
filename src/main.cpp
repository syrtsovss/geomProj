#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

// Начальный цвет фона в RGBA
float color[3]={0, 0.007, 0.223};
// Цвет фона
static sf::Color bg;
// Задаём цвет фона
void setColor(float * newColor){
    bg.r=static_cast<unsigned char>(newColor[0]*255.f);
    bg.g=static_cast<unsigned char>(newColor[1]*255.f);
    bg.b=static_cast<unsigned char>(newColor[2]*255.f);
}


int main() {
    // Создаём окно
    sf::RenderWindow window(sf::VideoMode(1080, 720), "Geometry project");
    
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
        }
        // Запускаем обновление окна по таймеру с заданной частотой
        ImGui::SFML::Update(window, timer.restart());
        // Создаём окно управления
        ImGui::Begin("Control");
        // Выбор цвета
        if (ImGui::ColorEdit3("Background color", color)) {
            // Если значение color изменилось, меняем цвет фона
            setColor(color);
        }
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