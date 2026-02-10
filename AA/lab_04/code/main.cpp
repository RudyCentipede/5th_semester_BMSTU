#include "cpu_time.h"
#include "graph_dbscan.h"

int main() {
  int M, minPts, threads;
  char dir;
  bool directed;
  int choice;

  cout << "=============================================\n";
  cout << "   Кластеризация графа по алгоритму DBSCAN\n";
  cout << "=============================================\n";
  cout << "\nВыберите режим:\n";
  cout << "  1. Последовательный DBSCAN\n";
  cout << "  2. Параллельный DBSCAN\n";
  cout << "  3. Массовые замеры (последовательный vs параллельный с 1 "
          "потоком)\n";
  cout << "  4. Замеры масштабирования по числу потоков (1,2,4,...,8q)\n";
  cout << "Ваш выбор: ";

  cin >> choice;
  cout << "=============================================\n";

  cout << "Введите параметр M: ";
  cin >> M;
  cout << "Введите minPts: ";
  cin >> minPts;
  cout << "Граф ориентированный? (y/n): ";
  cin >> dir;
  directed = (dir == 'y' || dir == 'Y');

  switch (choice) {
  case 1: {
    cout << "Введите путь к .dot файлу: ";
    string filename;
    cin >> filename;
    dbscan(filename, M, minPts, directed, true);
    break;
  }
  case 2: {
    cout << "Введите путь к .dot файлу: ";
    string filename;
    cin >> filename;
    cout << "Введите количество потоков: ";
    cin >> threads;
    dbscan_parallel(filename, M, minPts, threads, directed, true);
    break;
  }
  case 3: {
    cpu_time_single_parallel(M, minPts, directed);
    break;
  }
  case 4: {
    cpu_time_parallel(M, minPts, directed);
    break;
  }
  default:
    cout << "Неверный выбор!\n";
  }

  cout << "=============================================\n";
  return 0;
}
