using System.Text;
using System.Text.Encodings.Web;
using System.Text.Unicode;

namespace JSON;

using System.Data;
using Npg1sql;
using System.Text.Json;
using System.Text.Json.Serialization;

public class Book
{
    [JsonPropertyName("id")] public int Id { get; set; }

    [JsonPropertyName("title")] public string Title { get; set; } = "";

    [JsonPropertyName("genre")] public string Genre { get; set; } = "";

    [JsonPropertyName("publication_year")] public int PublicationYear { get; set; }

    [JsonPropertyName("publisher")] public string Publisher { get; set; } = "";

    [JsonPropertyName("page_count")] public int PageCount { get; set; }
}

public class Program
{
    static string _connectionString = "Host=localhost;Database=postgres;Username=postgres;Password=postgres";
    static string _jsonFilePath = "D:\\DataBase\\DB\\lab_07\\DBApp\\JSON\\library.json";

    public static void CreateJsonFromDatabase()
    {
        try
        {
            using var connection = new NpgsqlConnection(_connectionString);
            connection.Open();

            var query = @"
                SELECT 
                    b.id, 
                    b.title, 
                    g.name as genre,
                    b.publication_year,
                    b.publisher,
                    b.page_count
                FROM tp.books b
                JOIN tp.genres g ON b.genre_id = g.id
                ORDER BY b.id";

            var booksData = new List<Dictionary<string, object>>();

            using (var command = new NpgsqlCommand(query, connection))
            using (var reader = command.ExecuteReader())
            {
                while (reader.Read())
                {
                    var book = new Dictionary<string, object>
                    {
                        ["id"] = reader.GetInt32("id"),
                        ["title"] = reader.GetString("title"),
                        ["genre"] = reader.GetString("genre"),
                        ["publication_year"] = reader.GetInt32("publication_year"),
                        ["publisher"] = reader.GetString("publisher"),
                        ["page_count"] = reader.GetInt32("page_count")
                    };
                    booksData.Add(book);
                }
            }

            var jsonData = new Dictionary<string, object>
            {
                ["database"] = "PostgreSQL",
                ["schema"] = "tp",
                ["last_updated"] = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"),
                ["total_books"] = booksData.Count,
                ["books"] = booksData
            };

            var options = new JsonSerializerOptions
            {
                WriteIndented = true,
                Encoder = JavaScriptEncoder.Create(UnicodeRanges.All),
                PropertyNamingPolicy = JsonNamingPolicy.CamelCase
            };

            var json = JsonSerializer.Serialize(jsonData, options);

            File.WriteAllText(_jsonFilePath, json, System.Text.Encoding.UTF8);
            Console.WriteLine($"JSON документ создан из БД. Записей: {booksData.Count}");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Ошибка подключения к БД: {ex.Message}");
        }
    }

    public static void UpdateBookInJson()
    {
        try
        {
            if (!File.Exists(_jsonFilePath))
            {
                Console.WriteLine("JSON файл не существует");
                return;
            }

            string json = File.ReadAllText(_jsonFilePath, Encoding.UTF8);

            var options = new JsonSerializerOptions
            {
                WriteIndented = true,
                Encoder = JavaScriptEncoder.Create(UnicodeRanges.All),
                PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
                PropertyNameCaseInsensitive = true
            };

            using JsonDocument document = JsonDocument.Parse(json);
            JsonElement root = document.RootElement;

            if (!root.TryGetProperty("books", out JsonElement booksElement))
            {
                Console.WriteLine("Некорректный JSON: нет свойства 'books'");
                return;
            }

            var books = JsonSerializer.Deserialize<List<Book>>(booksElement.GetRawText(), options)
                        ?? new List<Book>();

            Console.WriteLine("\n=== ОБНОВЛЕНИЕ КНИГИ В JSON ===");
            Console.Write("Введите ID книги, которую нужно изменить: ");

            if (!int.TryParse(Console.ReadLine(), out int id))
            {
                Console.WriteLine("Некорректный ID.");
                return;
            }

            var book = (from b in books
                where b.Id == id
                select b).FirstOrDefault();

            if (book == null)
            {
                Console.WriteLine($"Книга с ID {id} не найдена.");
                return;
            }

            Console.WriteLine($"\nТекущие данные книги (ID {book.Id}):");
            Console.WriteLine($"Название: {book.Title}");
            Console.WriteLine($"Жанр: {book.Genre}");
            Console.WriteLine($"Год издания: {book.PublicationYear}");
            Console.WriteLine($"Издатель: {book.Publisher}");
            Console.WriteLine($"Страниц: {book.PageCount}");

            Console.WriteLine("\nОставьте поле пустым, чтобы не менять значение.");

            Console.Write("Новое название: ");
            string newTitle = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(newTitle))
                book.Title = newTitle.Trim();

            Console.Write("Новый жанр: ");
            string newGenre = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(newGenre))
                book.Genre = newGenre.Trim();

            Console.Write("Новый год издания: ");
            string yearInput = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(yearInput) &&
                int.TryParse(yearInput, out int newYear) &&
                newYear >= 1500 && newYear <= DateTime.Now.Year)
            {
                book.PublicationYear = newYear;
            }

            Console.Write("Новый издатель: ");
            string newPublisher = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(newPublisher))
                book.Publisher = newPublisher.Trim();

            Console.Write("Новое количество страниц: ");
            string pagesInput = Console.ReadLine();
            if (!string.IsNullOrWhiteSpace(pagesInput) &&
                int.TryParse(pagesInput, out int newPageCount) &&
                newPageCount > 0)
            {
                book.PageCount = newPageCount;
            }

            string database = root.TryGetProperty("database", out var dbProp) ? dbProp.GetString() : "PostgreSQL";
            string schema = root.TryGetProperty("schema", out var schemaProp) ? schemaProp.GetString() : "tp";

            var newRoot = new Dictionary<string, object>
            {
                ["database"] = database,
                ["schema"] = schema,
                ["last_updated"] = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"),
                ["total_books"] = books.Count,
                ["books"] = books
            };

            string updatedJson = JsonSerializer.Serialize(newRoot, options);
            File.WriteAllText(_jsonFilePath, updatedJson, Encoding.UTF8);

            Console.WriteLine("\nКнига успешно обновлена в JSON!");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Ошибка обновления JSON: {ex.Message}");
        }
    }


    public static void ReadBookById()
    {
        try
        {
            if (!File.Exists(_jsonFilePath))
            {
                Console.WriteLine("JSON файл не существует. Сначала создайте документ из БД.");
                return;
            }

            string json = File.ReadAllText(_jsonFilePath, System.Text.Encoding.UTF8);

            using JsonDocument document = JsonDocument.Parse(json);
            JsonElement root = document.RootElement;

            Console.WriteLine("\n=== ПОИСК КНИГИ ПО ID ===");
            Console.Write("Введите ID книги: ");

            if (int.TryParse(Console.ReadLine(), out int bookId))
            {
                var booksArray = root.GetProperty("books");
                var bookFound = false;

                foreach (var bookElement in booksArray.EnumerateArray())
                {
                    int currentId = bookElement.GetProperty("id").GetInt32();

                    if (currentId == bookId)
                    {
                        Console.WriteLine("\nНайдена книга:");
                        Console.WriteLine($"ID: {currentId}");
                        Console.WriteLine($"Название: {bookElement.GetProperty("title")}");
                        Console.WriteLine($"Жанр: {bookElement.GetProperty("genre")}");
                        Console.WriteLine($"Год издания: {bookElement.GetProperty("publication_year").GetInt32()}");
                        Console.WriteLine($"Издатель: {bookElement.GetProperty("publisher")}");
                        Console.WriteLine($"Количество страниц: {bookElement.GetProperty("page_count").GetInt32()}");
                        bookFound = true;
                    }
                }

                if (!bookFound)
                    Console.WriteLine($"Книга с ID {bookId} не найдена.");
            }
            else
                Console.WriteLine("Некорректный ID. Введите число.");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Ошибка чтения JSON: {ex.Message}");
        }
    }

    public static void AddNewBook()
    {
        try
        {
            if (!File.Exists(_jsonFilePath))
            {
                Console.WriteLine("JSON файл не существует");
                return;
            }

            string json = File.ReadAllText(_jsonFilePath, Encoding.UTF8);

            var options = new JsonSerializerOptions
            {
                WriteIndented = true,
                Encoder = JavaScriptEncoder.Create(UnicodeRanges.All),
                PropertyNamingPolicy = JsonNamingPolicy.CamelCase
            };

            var jsonData = JsonSerializer.Deserialize<Dictionary<string, JsonElement>>(json, options);

            if (jsonData == null || !jsonData.ContainsKey("books"))
            {
                Console.WriteLine("Некорректный JSON: нет свойства 'books'");
                return;
            }

            var booksJson = jsonData["books"].GetRawText();
            var books = JsonSerializer.Deserialize<List<Dictionary<string, JsonElement>>>(booksJson, options)
                        ?? new List<Dictionary<string, JsonElement>>();

            Console.WriteLine("\n=== ДОБАВЛЕНИЕ НОВОЙ КНИГИ ===");

            Console.Write("Введите название книги: ");
            string title = Console.ReadLine()?.Trim();
            if (string.IsNullOrWhiteSpace(title))
                title = "Без названия";

            Console.Write("Введите жанр: ");
            string genre = Console.ReadLine()?.Trim();
            if (string.IsNullOrWhiteSpace(genre))
                genre = "Не указан";

            Console.Write("Введите год издания: ");
            if (!int.TryParse(Console.ReadLine(), out int year) || year < 1500 || year > DateTime.Now.Year)
            {
                year = DateTime.Now.Year;
                Console.WriteLine($"Используется текущий год: {year}");
            }

            Console.Write("Введите издателя: ");
            string publisher = Console.ReadLine()?.Trim();
            if (string.IsNullOrWhiteSpace(publisher))
                publisher = "Не указан";

            Console.Write("Введите количество страниц: ");
            if (!int.TryParse(Console.ReadLine(), out int pageCount) || pageCount <= 0)
            {
                pageCount = 100;
                Console.WriteLine($"Используется значение по умолчанию: {pageCount}");
            }

            int maxId = 0;
            foreach (var book in books)
            {
                if (book.TryGetValue("id", out JsonElement idElement) &&
                    idElement.ValueKind == JsonValueKind.Number)
                {
                    int currentId = idElement.GetInt32();
                    if (currentId > maxId)
                        maxId = currentId;
                }
            }

            var newBook = new Dictionary<string, object>
            {
                ["id"] = maxId + 1,
                ["title"] = title,
                ["genre"] = genre,
                ["publication_year"] = year,
                ["publisher"] = publisher,
                ["page_count"] = pageCount
            };

            var updatedBooks = books
                .Select(b => new Dictionary<string, object>
                {
                    ["id"] = b["id"].GetInt32(),
                    ["title"] = b["title"].GetString(),
                    ["genre"] = b["genre"].GetString(),
                    ["publication_year"] = b["publication_year"].GetInt32(),
                    ["publisher"] = b["publisher"].GetString(),
                    ["page_count"] = b["page_count"].GetInt32(),
                })
                .ToList();

            updatedBooks.Add(newBook);

            var newRoot = new Dictionary<string, object>
            {
                ["books"] = updatedBooks,
                ["totalBooks"] = updatedBooks.Count,
                ["lastUpdated"] = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss")
            };

            string updatedJson = JsonSerializer.Serialize(newRoot, options);
            File.WriteAllText(_jsonFilePath, updatedJson, Encoding.UTF8);

            Console.WriteLine("\nКнига успешно добавлена!");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Ошибка добавления в JSON: {ex.Message}");
        }
    }


    public static void Main(string[] args)
    {
        int choise;
        bool flag = true;
        //CreateJsonFromDatabase();
        while (flag)
        {
            Console.WriteLine("\nВыберите номер пункта меню:");
            Console.WriteLine("1. Чтение из JSON документа");
            Console.WriteLine("2. Обновление JSON  документа.");
            Console.WriteLine("3. Запись в JSON  документ.");
            Console.WriteLine("0. Выход из программы.\n");
            choise = int.Parse(Console.ReadLine());
            switch (choise)
            {
                case 0:
                    flag = false;
                    break;
                case 1:
                    ReadBookById();
                    break;
                case 2:
                    UpdateBookInJson();
                    break;
                case 3:
                    AddNewBook();
                    break;
            }
        }
    }
}