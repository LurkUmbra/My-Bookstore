#include <fstream>
#include <iostream>
#include <string>

struct Student { int id; char name[32]; };

int main() {
  {
    std::fstream file("student.bin",
      std::ios::out | std::ios::binary | std::ios::trunc);
    for (int i = 0; i < 3; i++) {
      std::string tmp = "Student_" + std::to_string(i + 1);
      Student s{};
      s.id = i + 1;
      std::copy(tmp.begin(), tmp.end(), s.name);
      s.name[tmp.size()] = '\0';
      file.seekp(i * sizeof(Student), std::ios::beg);
      file.write(reinterpret_cast<char*>(&s), sizeof(Student));
    }
  }

  std::fstream file("student.bin", std::ios::in | std::ios::binary);
  Student s{};
  file.seekg(1 * sizeof(Student), std::ios::beg);
  file.read(reinterpret_cast<char*>(&s), sizeof(Student));
  std::cout << s.id << ' ' << s.name << std::endl;
  file.close();
  return 0;
}
