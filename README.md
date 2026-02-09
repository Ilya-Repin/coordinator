# Координатор для системы доставки сообщений
Сервис координатор – управляет кластером хабов

## Краткое описание
- Базовая сущность в системе – канал (через канал проходят сообщения, например одному читателю или группе)
- Шлюзы держат подключения к читателям
- Хабы обслуживают потоки сообщений назначенных партиций
- Каналы разделены на партиции
- Партиции каналов представлены как диапазоны на хеш-кольце
- Среди координаторов выбирается лидер
- Лидер раз в N секунд проводит координацию:
    1) Определяет текущее состояние кластера
    2) Распределяет и балансирует партиции между хабами
    3) Публикует новую карту партциий, которая содержит маппинг партиций в хабы и номер текущей эпохи
- Leader Election, Service Discovery и публикация partition map реализованы через механизм распределенных семафоров узлов координации YDB
- Параметры балансировки можно крутить через динамический конфиг
- Прототип ["хабошлюза"](https://github.com/Repin-Daniil/chat-service)

## Makefile

`PRESET` is either `debug`, `release`, or if you've added custom presets in `CMakeUserPresets.json`, it
can also be `debug-custom`, `release-custom`.

* `make cmake-PRESET` - run cmake configure, update cmake options and source file lists
* `make build-PRESET` - build the service
* `make test-PRESET` - build the service and run all tests
* `make start-PRESET` - build the service, start it in testsuite environment and leave it running
* `make install-PRESET` - build the service and install it in directory set in environment `PREFIX`
* `make` or `make all` - build and run all tests in `debug` and `release` modes
* `make format` - reformat all C++ and Python sources
* `make dist-clean` - clean build files and cmake cache
* `make docker-COMMAND` - run `make COMMAND` in docker environment
* `make docker-clean-data` - stop docker containers


## License

The original template is distributed under the [Apache-2.0 License](https://github.com/userver-framework/userver/blob/develop/LICENSE)
and [CLA](https://github.com/userver-framework/userver/blob/develop/CONTRIBUTING.md). Services based on the template may change
the license and CLA.
