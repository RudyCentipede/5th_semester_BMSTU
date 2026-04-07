using System;
using System.Collections.Generic;
using System.Linq;

public class Genre
{
    public int Id { get; set; }
    public string? Name { get; set; }
}

public class Author
{
    public int Id { get; set; }
    public string? FirstName { get; set; }
    public string? LastName { get; set; }
    public DateTime? BirthDate { get; set; }
    public string? Country { get; set; }
    public string? Biography { get; set; }
}

public class Book
{
    public int Id { get; set; }
    public string? Title { get; set; }
    public int GenreId { get; set; }
    public int PublicationYear { get; set; }
    public string? Publisher { get; set; }
    public int PageCount { get; set; }
}

public class Reader
{
    public int Id { get; set; }
    public string? Surname { get; set; }
    public string? Name { get; set; }
    public string? MiddleName { get; set; }
    public string? Address { get; set; }
    public string? Sex { get; set; }
    public DateTime? Birthday { get; set; }
    public string? Email { get; set; }
    public string? Phone { get; set; }
    public DateTime RegistrationDate { get; set; }
}

public class BookLoan
{
    public int Id { get; set; }
    public int BookId { get; set; }
    public int AuthorId { get; set; }
    public int ReaderId { get; set; }
    public DateTime LoanDate { get; set; }
    public DateTime DueDate { get; set; }
    public DateTime? ReturnDate { get; set; }
    public string? Status { get; set; }
}

public class Program
{
    static string _connectionString = "Host=localhost;Database=postgres;Username=postgres;Password=postgres";
    static string _jsonFilePath = "D:\\DataBase\\DB\\lab_07\\DBApp\\JSON\\library.json";

    private static List<Book> LoadBooks()
    {
        var result = new List<Book>();

        using var connection = new Npgsql.NpgsqlConnection(_connectionString);
        connection.Open();

        var sql = @"SELECT id, title, genre_id, publication_year, publisher, page_count
                FROM tp.books";

        using var cmd = new Npgsql.NpgsqlCommand(sql, connection);
        using var reader = cmd.ExecuteReader();
        while (reader.Read())
        {
            result.Add(new Book
            {
                Id = reader.GetInt32(reader.GetOrdinal("id")),
                Title = reader.GetString(reader.GetOrdinal("title")),
                GenreId = reader.GetInt32(reader.GetOrdinal("genre_id")),
                PublicationYear = reader.GetInt32(reader.GetOrdinal("publication_year")),
                Publisher = reader.GetString(reader.GetOrdinal("publisher")),
                PageCount = reader.GetInt32(reader.GetOrdinal("page_count"))
            });
        }

        return result;
    }

    private static List<Genre> LoadGenres()
    {
        var result = new List<Genre>();

        using var connection = new Npgsql.NpgsqlConnection(_connectionString);
        connection.Open();

        var sql = @"SELECT id, name FROM tp.genres";

        using var cmd = new Npgsql.NpgsqlCommand(sql, connection);
        using var reader = cmd.ExecuteReader();
        while (reader.Read())
        {
            result.Add(new Genre
            {
                Id = reader.GetInt32(reader.GetOrdinal("id")),
                Name = reader.GetString(reader.GetOrdinal("name"))
            });
        }

        return result;
    }

    private static List<Author> LoadAuthors()
    {
        var result = new List<Author>();

        using var connection = new Npgsql.NpgsqlConnection(_connectionString);
        connection.Open();

        var sql = @"SELECT id, first_name, last_name, birth_date, country, biography
                FROM tp.authors";

        using var cmd = new Npgsql.NpgsqlCommand(sql, connection);
        using var reader = cmd.ExecuteReader();
        while (reader.Read())
        {
            result.Add(new Author
            {
                Id = reader.GetInt32(reader.GetOrdinal("id")),
                FirstName = reader.GetString(reader.GetOrdinal("first_name")),
                LastName = reader.GetString(reader.GetOrdinal("last_name")),
                BirthDate = reader.IsDBNull(reader.GetOrdinal("birth_date"))
                    ? null
                    : reader.GetDateTime(reader.GetOrdinal("birth_date")),
                Country = reader.IsDBNull(reader.GetOrdinal("country"))
                    ? null
                    : reader.GetString(reader.GetOrdinal("country")),
                Biography = reader.IsDBNull(reader.GetOrdinal("biography"))
                    ? null
                    : reader.GetString(reader.GetOrdinal("biography"))
            });
        }

        return result;
    }

    public static void SingleTableLinqQuery()
    {
        var books = LoadBooks();

        var query =
            from b in books
            where b.PublicationYear >= 2010
            orderby b.Title
            select b;

        Console.WriteLine("\n=== Книги после 2010 года ===");
        foreach (var b in query)
        {
            Console.WriteLine($"{b.Id}: {b.Title} ({b.PublicationYear}), стр.: {b.PageCount}");
        }
    }

    public static void MultiTableLinqQuery()
    {
        var books = LoadBooks();
        var genres = LoadGenres();

        var query =
            from b in books
            join g in genres on b.GenreId equals g.Id
            orderby g.Name, b.Title
            select new
            {
                BookId = b.Id,
                b.Title,
                Genre = g.Name,
                b.PublicationYear
            };

        Console.WriteLine("\n=== Книги с жанрами ===");
        foreach (var item in query)
        {
            Console.WriteLine($"{item.BookId}: {item.Title} — {item.Genre}, {item.PublicationYear} г.");
        }
    }

    public static void InsertAuthor()
{
    Console.WriteLine("\n=== ДОБАВЛЕНИЕ АВТОРА ===");
    
    Console.Write("Имя: ");
    string? firstName = Console.ReadLine();
    if (string.IsNullOrWhiteSpace(firstName))
    {
        Console.WriteLine("Имя обязательно.");
        return;
    }

    Console.Write("Фамилия: ");
    string? lastName = Console.ReadLine();
    if (string.IsNullOrWhiteSpace(lastName))
    {
        Console.WriteLine("Фамилия обязательна.");
        return;
    }
    
    Console.Write("Дата рождения (ГГГГ-ММ-ДД, можно оставить пустым): ");
    string? birthInput = Console.ReadLine();
    DateTime? birthDate = null;
    if (!string.IsNullOrWhiteSpace(birthInput))
    {
        if (DateTime.TryParse(birthInput, out var parsed))
        {
            birthDate = parsed;
        }
        else
        {
            Console.WriteLine("Некорректная дата, будет записано NULL.");
        }
    }

    Console.Write("Страна (можно оставить пустым): ");
    string? country = Console.ReadLine();
    if (string.IsNullOrWhiteSpace(country))
        country = null;

    Console.Write("Биография (можно оставить пустым): ");
    string? biography = Console.ReadLine();
    if (string.IsNullOrWhiteSpace(biography))
        biography = null;

    using var connection = new Npgsql.NpgsqlConnection(_connectionString);
    connection.Open();
    
    int newId;
    using (var cmdMax = new Npgsql.NpgsqlCommand("SELECT COALESCE(MAX(id), 0) + 1 FROM tp.authors", connection))
    {
        newId = Convert.ToInt32(cmdMax.ExecuteScalar());
    }

    var sql = @"
        INSERT INTO tp.authors (id, first_name, last_name, birth_date, country, biography)
        VALUES (@id, @fn, @ln, @birth_date, @country, @biography);";

    using var cmd = new Npgsql.NpgsqlCommand(sql, connection);
    cmd.Parameters.AddWithValue("id", newId);
    cmd.Parameters.AddWithValue("fn", firstName.Trim());
    cmd.Parameters.AddWithValue("ln", lastName.Trim());
    
    if (birthDate.HasValue)
        cmd.Parameters.AddWithValue("birth_date", birthDate.Value);
    else
        cmd.Parameters.AddWithValue("birth_date", DBNull.Value);

    cmd.Parameters.AddWithValue("country", (object?)country ?? DBNull.Value);
    cmd.Parameters.AddWithValue("biography", (object?)biography ?? DBNull.Value);

    int rows = cmd.ExecuteNonQuery();
    Console.WriteLine(rows > 0 ? "Автор добавлен." : "Автор не добавлен.");
}


    public static void UpdateBookPages()
    {
        Console.WriteLine("\n=== ОБНОВЛЕНИЕ КОЛИЧЕСТВА СТРАНИЦ ===");
        Console.Write("Введите ID книги: ");

        if (!int.TryParse(Console.ReadLine(), out int id))
        {
            Console.WriteLine("Некорректный ID");
            return;
        }

        Console.Write("Новое количество страниц: ");
        if (!int.TryParse(Console.ReadLine(), out int newPages) || newPages <= 0)
        {
            Console.WriteLine("Некорректное значение страниц");
            return;
        }

        using var connection = new Npgsql.NpgsqlConnection(_connectionString);
        connection.Open();

        var sql = @"UPDATE tp.books
                SET page_count = @pages
                WHERE id = @id";

        using var cmd = new Npgsql.NpgsqlCommand(sql, connection);
        cmd.Parameters.AddWithValue("pages", newPages);
        cmd.Parameters.AddWithValue("id", id);

        int rows = cmd.ExecuteNonQuery();
        Console.WriteLine(rows > 0 ? "Книга обновлена." : "Книга с таким ID не найдена.");
    }

    public static void DeleteReader()
    {
        Console.WriteLine("\n=== УДАЛЕНИЕ ЧИТАТЕЛЯ ===");
        Console.Write("Введите ID читателя: ");

        if (!int.TryParse(Console.ReadLine(), out int id))
        {
            Console.WriteLine("Некорректный ID");
            return;
        }

        using var connection = new Npgsql.NpgsqlConnection(_connectionString);
        connection.Open();

        var sql = @"DELETE FROM tp.readers WHERE id = @id";

        using var cmd = new Npgsql.NpgsqlCommand(sql, connection);
        cmd.Parameters.AddWithValue("id", id);

        int rows = cmd.ExecuteNonQuery();
        Console.WriteLine(rows > 0 ? "Читатель удалён." : "Читатель с таким ID не найден.");
    }

    public static void CallStoredProcedure()
    {
        Console.WriteLine("\n=== ВЫДАЧИ КНИГ ЧИТАТЕЛЯ (через хранимую процедуру) ===");
        Console.Write("Введите ID читателя: ");

        if (!int.TryParse(Console.ReadLine(), out int readerId))
        {
            Console.WriteLine("Некорректный ID");
            return;
        }

        using var connection = new Npgsql.NpgsqlConnection(_connectionString);
        connection.Open();
        
        var sql = @"SELECT * FROM tp.get_loans_by_reader(@rid)";

        using var cmd = new Npgsql.NpgsqlCommand(sql, connection);
        cmd.Parameters.AddWithValue("rid", readerId);

        using var reader = cmd.ExecuteReader();

        if (!reader.HasRows)
        {
            Console.WriteLine("Выдач для этого читателя не найдено.");
            return;
        }

        while (reader.Read())
        {
            int loanId = reader.GetInt32(reader.GetOrdinal("loan_id"));
            string title = reader.GetString(reader.GetOrdinal("book_title"));
            DateTime loanDate = reader.GetDateTime(reader.GetOrdinal("loan_date"));
            DateTime dueDate = reader.GetDateTime(reader.GetOrdinal("due_date"));
            DateTime? returnDate = reader.IsDBNull(reader.GetOrdinal("return_date"))
                ? null
                : reader.GetDateTime(reader.GetOrdinal("return_date"));
            string status = reader.GetString(reader.GetOrdinal("status"));

            Console.WriteLine($"Выдача {loanId}: \"{title}\"");
            Console.WriteLine($"  Дата выдачи: {loanDate:yyyy-MM-dd}");
            Console.WriteLine($"  Вернуть до: {dueDate:yyyy-MM-dd}");
            Console.WriteLine(
                $"  Факт возврата: {(returnDate.HasValue ? returnDate.Value.ToString("yyyy-MM-dd") : "ещё не возвращена")}");
            Console.WriteLine($"  Статус: {status}");
        }
    }


    public static void Main(string[] args)
    {
        int choise;
        bool flag = true;

        while (flag)
        {
            Console.WriteLine("\nВыберите номер пункта меню:");
            Console.WriteLine("1. Однотабличный запрос");
            Console.WriteLine("2. Многотабличный запрос");
            Console.WriteLine("3. INSERT автора");
            Console.WriteLine("4. UPDATE книги");
            Console.WriteLine("5. DELETE читателя");
            Console.WriteLine("6. Хранимая процедура (выдачи читателя)");
            Console.WriteLine("0. Выход из программы.\n");

            if (!int.TryParse(Console.ReadLine(), out choise))
                continue;

            switch (choise)
            {
                case 0:
                    flag = false;
                    break;
                case 1:
                    SingleTableLinqQuery();
                    break;
                case 2:
                    MultiTableLinqQuery();
                    break;
                case 3:
                    InsertAuthor();
                    break;
                case 4:
                    UpdateBookPages();
                    break;
                case 5:
                    DeleteReader();
                    break;
                case 6:
                    CallStoredProcedure();
                    break;
            }
        }
    }
}
