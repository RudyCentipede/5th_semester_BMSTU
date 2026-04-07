using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.ConstrainedExecution;
using System.Text;
namespace DBApp;

public class Book
{
    public int Id { get; set; }
    public required string Title { get; set; }
    public int GenreId { get; set; }
    public int PublicationYear { get; set; }
    public required string Publisher { get; set; }
    public int PageCount { get; set; }
    
    public static List<Book> LoadBooksFromCsv(string filePath)
    {
        var books = new List<Book>();
    
        try
        {
            var lines = File.ReadAllLines(filePath, Encoding.UTF8);
            
            for (int i = 1; i < lines.Length; i++)
            {
                var columns = ParseCsvLine(lines[i]);

                var book = new Book
                {
                    Id = int.Parse(columns[0].Trim()),
                    Title = columns[1].Trim(),
                    GenreId = int.Parse(columns[2].Trim()),
                    PublicationYear = int.Parse(columns[3].Trim()),
                    Publisher = columns[4].Trim(),
                    PageCount = int.Parse(columns[5].Trim())
                };
                books.Add(book);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Ошибка загрузки данных: {ex.Message}");
        }
    
        return books;
    }

    private static string[] ParseCsvLine(string line)
    {
        var result = new List<string>();
        var current = new StringBuilder();
        bool inQuotes = false;
    
        for (int i = 0; i < line.Length; i++)
        {
            char c = line[i];
        
            if (c == '"') 
                if (inQuotes && i + 1 < line.Length && line[i + 1] == '"')
                {
                    current.Append('"');
                    i++;
                }
                else
                    inQuotes = !inQuotes; 
            
            else if (c == ',' && !inQuotes)
            {
                result.Add(current.ToString());
                current.Clear();
            }
            else
                current.Append(c);
        }
        
        result.Add(current.ToString());
    
        return result.ToArray();
    }
}

public class Genre
{
    public int Id { get; set; }
    public string Name { get; set; }
    
    public static List<Genre> LoadGenresFromCsv(string filePath)
    {
        var genres = new List<Genre>();
    
        try
        {
            var lines = File.ReadAllLines(filePath, Encoding.UTF8);
            
            for (int i = 1; i < lines.Length; i++)
            {
                var columns = lines[i].Split(',');

                var genre = new Genre
                {
                    Id = int.Parse(columns[0].Trim()),
                    Name = columns[1].Trim(),
                };
                
                genres.Add(genre);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Ошибка загрузки данных: {ex.Message}");
        }
    
        return genres;
    }
}

class Program()
{
    static void Main(string[] args)
    {
        string booksPath = "D:\\DataBase\\DB\\csv_data\\books.csv";
        string genresPath = "D:\\DataBase\\DB\\csv_data\\genres.csv";

        var books = Book.LoadBooksFromCsv(booksPath);
        var genres = Genre.LoadGenresFromCsv(genresPath);
        
        Console.WriteLine("1. Книги после 2000 года:");
        var modernBooks = from b in books
            where b.PublicationYear > 2000
            let decade = (b.PublicationYear / 10) * 10
            orderby b.PublicationYear descending
            select new { 
                b.Title, 
                Year = b.PublicationYear,
                Decade = $"{decade}-е",
                b.Publisher,
                Pages = b.PageCount
            };
        
        foreach (var book in modernBooks.Take(5))
            Console.WriteLine($"{book.Title} ({book.Year}, {book.Decade}) - {book.Publisher} - {book.Pages} стр.");
        
        
        Console.WriteLine("\n2. Издательства с наибольшим количеством книг:");
        var maxCount = books.GroupBy(b => b.Publisher).Max(g => g.Count());
        
        var publisherStats = from b in books
            group b by b.Publisher into publisherGroup
            where publisherGroup.Count() == maxCount
            let avgPages = publisherGroup.Average(b => b.PageCount)
            let maxYear = publisherGroup.Max(b => b.PublicationYear)
            orderby publisherGroup.Count() descending
            select new {
                Publisher = publisherGroup.Key,
                BookCount = publisherGroup.Count(),
                AvgPages = Math.Round(avgPages, 1),
            };
        
        foreach (var stat in publisherStats)
            Console.WriteLine($"{stat.Publisher}: {stat.BookCount} книг, ср. {stat.AvgPages} стр.");
        
        Console.WriteLine("\n3. Книги жанра Фантастика:");
        var techBooks = from b in books
            join g in genres on b.GenreId equals g.Id
            where g.Id == 2
            orderby b.PublicationYear descending
            select new { 
                b.Title, 
                Year = b.PublicationYear,
                b.Publisher
            };
        
        foreach (var book in techBooks.Take(5))
            Console.WriteLine($"{book.Title} - ({book.Year}) // {book.Publisher}");
        
        Console.WriteLine("\n5. Книги XIX века с категориями по объёму:");
        var classicLargeBooks = from b in books
            where b.PublicationYear >= 1800 && b.PublicationYear < 1900
            let century = "XIX век"
            let sizeCategory = b.PageCount > 800 ? "Крупная" : b.PageCount > 500 ? "Средняя" : "Компактная"
            orderby b.PageCount descending, b.PublicationYear
            select new {
                b.Title,
                Year = b.PublicationYear,
                Century = century,
                Pages = b.PageCount,
                Size = sizeCategory,
                b.Publisher
            };
        
        foreach (var book in classicLargeBooks.Take(5))
            Console.WriteLine($"{book.Title} ({book.Year}, {book.Century}) - {book.Pages} стр. ({book.Size}) - {book.Publisher}");
        
        Console.WriteLine("\n4. Изданные жанры книг после 2020 года:");
        var genrePageStats = from b in books
            join g in genres on b.GenreId equals g.Id
            where b.PublicationYear > 2020
            group b by g.Name into genreGroup
            let avgPages = genreGroup.Average(b => b.PageCount)
            orderby avgPages descending
            select new {
                Genre = genreGroup.Key,
                BookCount = genreGroup.Count(),
                AvgPages = (int)avgPages,
                MaxPages = genreGroup.Max(b => b.PageCount)
            };
            
        
        foreach (var book in genrePageStats.Take(5))
            Console.WriteLine($"{book.Genre}: {book.BookCount} шт. - ср. {book.AvgPages} стр. - макс. {book.MaxPages} стр.");
        
        Console.WriteLine("\n\nУдалить базу данных? (y/n)");
        var key = Console.ReadKey();

        if (key.Key == ConsoleKey.Y)
        {
            books.Clear();
            genres.Clear();
            Console.WriteLine("\nБД удалена: " + books.Count + " " + genres.Count);
        }
        else
        {
            Console.WriteLine("\nУдаление отменено.");
        }
    }
}
