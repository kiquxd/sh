# sh

Miniature shell for Linux written in C with `PATH` lookup, `cd` support

## Task

Вам предлагается написать небольшой аналог `shell`, в [файле](src/template.c) подготовлены некоторые функции:

- в функции `parse` предлагается распарсить введенный промпт, сохранив результат в `dst`

- в функции `run` предлагается реализовать запуск распаршенного промпта

- не забудьте очистить выделенную память (предлагается сделать это в функции `free_tokens`)

Помимо этого, требуется поддержать отображение пути до текущей рабочей директории.

## Running shell

```bash
make run
```

## Running tests

```bash
mkdir build
cd build
cmake ..
cmake --build .
ctest --output-on-failure
```
