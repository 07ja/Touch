#include "controller.hpp"
#include "model.hpp"
#include "view.hpp"


int main() {
    // Initialize components
    Controller app;
    Model model;
    View view;

    app.init("Touch", 960, 540);
    view.init(app.getRenderer());

    // Main loop
    while (!app.getQuit()) {
        app.input(view, model);
        view.render(app.getWindow(), app.getRenderer(), model);
    }

    return 0;
}