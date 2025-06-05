#include "logger.h"

int main() {
    auto logger = LoggerBuilder()
            .setLevel(LogLevel::DEBUG)
            .addConsoleSink()
            .addFileSink("application.log", FileOutPut::Mode::Append)
            .addFileSink("application1.log", FileOutPut::Mode::Truncate)
            .build();


    logger->debug("Начало работы приложения");
    logger->info("Загружены конфигурационные параметры");
    logger->warning("Необязательный параметр не задан");

    for (int i = 0; i < 3; ++i) {
        logger->info("Обработка данных, итерация " + std::to_string(i + 1));

        if (i == 1) {
            logger->error("Ошибка ввода данных на итерации " + std::to_string(i + 1));
        }
    }

    try {
        throw std::runtime_error("Серьезная ошибка в бизнес-логике");
    } catch (const std::exception &e) {
        logger->critical(e.what());
    }

    logger->info("Завершение работы приложения");


    return 0;
}