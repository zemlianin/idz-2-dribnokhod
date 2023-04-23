# ИДЗ-2 Дрибноход Евгений БПИ-217
Вариант 35
## About
Проект представляет из себя лабороторную работу решенную на 6 баллов

## Формулировка задачи: 

Задача для агронома. Председатель дачного кооператива Сидоров В.И. получил указание, что в связи с составлением единого
земельного кадастра, необходимо представить справку о площади
занимаемых земель. Известно, что территория с запада и востока
параллельна меридианам, на севере ограничены параллелью, а с
юга выходят к реке, описываемой функцией f(x). Требуется создать приложение, вычисляющее площадь угодий мето-
дом адаптивной квадратуры. Замечание: кривизну Земли изза малой занимаемой площади не учитывать. Количество процес-
сов, участвующих в вычислениях определяется опционально. Агроном нанимает счетоводов, каждый из которых обсчитывает неболь-
шой участок после чего добавляет свой результат к общей площади полученной в ходе проводимых расчетов. После этого агроном
назначает ему следующий свободный участок для проведения расчета. Аграном и каждый счетовод должны быть представлены
отдельными процессами.

## Инструкция к запуску
Для запуска программы, требуется перейти в папку с решением на соответветствующую оценку и запустить команду:
```
./manager.o <Number>
```
## Ввод:
Каждая из программ принимает на вход:
- Число-парметр командной строки, данный параметр задает число полос на которые будет делиться участок, также от этого параметр зависит число счетоводов и оно расчитывается по формуле:
  ```
   число_счетоводов = введенный_параметр / 2
  ```
  Число разбиений на полосы - влияет на точность
- Содержимое файла input.txt

  Содержимое файла состоит из трех чисел и задет параметры функции f(x), сама же функция f(x) задается формулой:
  ```
  abs(fun_param_1 * x*x*x + fun_param_2 * x*x + fun_param_3 * x);
  ```
## Вывод
  Каждая из программ осуществляет вывод:
  - В консоль, где каждый счетовод выводит значения границ только что измеренной им полосы, а также через двоеточие площадь этой полосы 
  - В файл: суммарное значение площади
  
## Взаимодействие сущностей
  1. Основной процесс запускает процессы счетоводов и те в соответствующей функции остонавливаются на первом семафоре.
  2. Оснвоной процесс расчитывает значения границ первой полосы
  3. Основной процесс записывает значения границ в соответсвующую разделяемую память
  4. Оснвной процесс высвобождает одного счетовода и остонавливается на втором семафоре
  5. Счетовод считывает значения границ, высвобождает поток менеджера и приступает к расчету
  6. Поток менеджера повторяет свою операцию по расчету границ/записи границ/ высвобождению одного счетовода/ ожидания дочитывания счетоводом 
  7. Поток счетовода дочитывает освобождает и менеджера и приступает к расчету
  8. Цикл продолжается до тех пор, пока менеджер не распилит всю облоасть на полосы
  9. Программа завержешается при досчитывании суммарной площади всех полос
