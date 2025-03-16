#include <iostream>
#include <vector>
#include <string>

using namespace std;

// Класс для хранения данных о книге
class Book {
public:
    int year;       // Год издания
    string title;   // Название книги
    string author;  // Автор

    // Конструктор для инициализации 
    Book(int y, const string& t, const string& a)
        : year(y), title(t), author(a) {}
};

// Статусы ячеек хеш-таблицы
enum CellStatus {
    EMPTY,    // Ячейка свободна
    OCCUPIED, // Ячейка занята
    DELETED   // Ячейка помечена как удалённая
};

// Эл-т хеш-таблицы
struct HashItem {
    Book data;      // Данные
    CellStatus status = EMPTY;

    // Конструктор по умолчанию (для пустых ячеек)
    HashItem() : data(0, "", "") {}
};

// Хеш-таблица с квадратичным пробированием
class HashTable {
private:
    vector<HashItem> table; // Вектор для хранения элементов (сама таблица)
    size_t sizeT;   // Размер таблицы
    size_t size;// Количество занятых элементов

    // Хеш-функция для книги
    size_t hashFunction(const Book& book) const {
        size_t titleHash = 0;
        // Полиномиальное хеширование для названия
        for (char c : book.title) {
            titleHash = (titleHash * 31 + c) % sizeT;
        }

        // Хеш для автора через стандартную функцию
        size_t authorHash = hash<string>{}(book.author) % sizeT;

        // Комбинируем хеши: (год + хеш_названия) XOR хеш_автора
        return ((book.year + titleHash) ^ authorHash) % sizeT;
    }

    // Метод квадратичного пробирования
    size_t quadraticProbing(size_t startIndex, size_t attempt) const {
        // Формула: startIndex + attempt² (модуль sizeT)
        return (startIndex + attempt * attempt) % sizeT;
    }

    // Проверка необходимости рехеширования
    bool needsRehash() const {
        // Рехешируем, если заполнено больше 70%
        return static_cast<double>(size) / sizeT >= 0.7;
    }

    // Рехеширование таблицы
    void rehash() {
        size_t newsizeT = sizeT * 2; // Удваиваем размер
        vector<HashItem> newTable(newsizeT); // Новая таблица

        // Переносим элементы из старой таблицы
        for (size_t i = 0; i < sizeT; ++i) {
            if (table[i].status == OCCUPIED) {
                const Book& book = table[i].data;
                size_t attempt = 0;
                size_t index;

                // Вставляем элемент в новую таблицу
                do {
                    size_t hash = hashFunction(book);
                    index = quadraticProbing(hash, attempt);
                    if (newTable[index].status != OCCUPIED) {
                        newTable[index].data = book;
                        newTable[index].status = OCCUPIED;
                        break;
                    }
                    attempt++;
                } while (attempt < newsizeT);
            }
        }

        // Заменяем старую таблицу новой
        table = move(newTable);
        sizeT = newsizeT;
    }

public:
    // Конструктор с начальным размером таблицы
    HashTable(size_t initialsizeT = 11)
        : sizeT(initialsizeT), size(0) {
        table.resize(sizeT); // Инициализируем вектор
    }

    // Вставка элемента
    bool insert(const Book& book) {
        if (needsRehash()) {
            rehash(); // Рехешируем при необходимости
        }

        size_t hash = hashFunction(book); // Начальный хеш
        size_t attempt = 0;
        size_t index;

        do {
            index = quadraticProbing(hash, attempt); // Новый индекс
            // Если ячейка свободна или удалена
            if (table[index].status != OCCUPIED) {
                table[index].data = book;     // Записываем данные
                table[index].status = OCCUPIED; // Помечаем как занятую
                size++; // Увеличиваем счётчик элементов
                return true; // Успешная вставка
            }
            attempt++;
        } while (attempt < sizeT); // Проверяем все возможные индексы

        cerr << "Ошибка: таблица переполнена!" << endl;
        return false;
    }

    // Поиск элемента
    Book* search(const Book& book) {
        size_t hash = hashFunction(book);
        size_t attempt = 0;
        size_t index;

        do {
            index = quadraticProbing(hash, attempt);
            // Если ячейка пуста, элемента нет
            if (table[index].status == EMPTY) {
                return nullptr;
            }
            // Если нашли совпадение по всем полям
            if (table[index].status == OCCUPIED &&
                table[index].data.title == book.title &&
                table[index].data.author == book.author &&
                table[index].data.year == book.year) {
                return &table[index].data; // Возвращаем указатель
            }
            attempt++;
        } while (attempt < sizeT);

        return nullptr; // Элемент не найден
    }

    // Удаление элемента
    bool remove(const Book& book) {
        size_t hash = hashFunction(book);
        size_t attempt = 0;
        size_t index;

        do {
            index = quadraticProbing(hash, attempt);
            if (table[index].status == EMPTY) {
                return false; // Элемент не найден
            }
            if (table[index].status == OCCUPIED &&
                table[index].data.title == book.title &&
                table[index].data.author == book.author &&
                table[index].data.year == book.year) {
                table[index].status = DELETED; // Помечаем как удалённый
                size--; // Уменьшаем счётчик
                return true;
            }
            attempt++;
        } while (attempt < sizeT);

        return false;
    }

    // Вывод содержимого таблицы
    void print() const {
        for (size_t i = 0; i < sizeT; ++i) {
            if (table[i].status == OCCUPIED) {
                cout << "[" << i << "] "
                    << table[i].data.title << " ("
                    << table[i].data.year << "), автор: "
                    << table[i].data.author << endl;
            }
        }
    }
};

// Пример использования
int main() {
    setlocale(LC_ALL, "Russian");
    HashTable ht; // Создаём таблицу

    // Создаём книги
    Book book1(1860, "война и мир", "Толстой");
    Book book2(1949, "1984", "George Orwell");
    Book book3(1925, "The Great Gatsby", "F. Scott Fitzgerald");

    // Вставляем книги в таблицу
    ht.insert(book1);
    ht.insert(book2);
    ht.insert(book3);
    

    cout << "Содержимое таблицы:" << endl;
    ht.print(); // Выводим данные

    // Поиск книги
    Book* found = ht.search(book2);
    if (found) {
        cout << "\nНайдена книга: " << found->title << endl;
    }

    // Удаление книги
    ht.remove(book3);
    cout << "\nПосле удаления 'The Great Gatsby':" << endl;
    ht.print();

    return 0;
}