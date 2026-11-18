# Closest-Pair-of-Points-Finder-Using-Divide-and-Conquer-in-C
# 📍 Closest Pair of Points Finder

### A Computational Geometry Project Using Divide and Conquer

A C-based graphical application that finds the closest pair of points in a 2D plane using **Divide and Conquer** and **Brute Force** algorithms. It visualizes the closest pair and compares the performance of both approaches.

---

## 📌 Project Overview

The Closest Pair of Points problem is a fundamental problem in Computational Geometry.

Given a set of points in a two-dimensional plane, the objective is to find the two points with the minimum Euclidean distance between them.

This project implements two approaches to solve the problem:

- **Brute Force:** Checks every possible pair of points to find the minimum distance.
- **Divide and Conquer:** Recursively divides the points into smaller subsets and efficiently finds the closest pair.

The application provides a graphical interface where users can generate random points, manually add points, execute algorithms, and compare their performance.

## ✨ Features

- **Interactive 2D Canvas:** Add points manually by clicking on the drawing area.
- **Random Point Generation:** Generate up to 1,000 random points.
- **Brute Force Algorithm:** Find the closest pair using exhaustive pairwise comparison.
- **Divide and Conquer Algorithm:** Find the closest pair using an optimized recursive approach.
- **Visual Highlighting:** Highlight the closest pair using a red connecting line.
- **Algorithm Comparison:** Execute both algorithms and compare their execution times.
- **Distance Calculation:** Display the minimum Euclidean distance between the closest points.
- **Clear Canvas:** Reset the canvas and start a new experiment.
- **Result Verification:** Verify that both algorithms produce the same minimum distance.

## 🧠 Algorithms Used

### 1. Brute Force Approach

The Brute Force algorithm calculates the Euclidean distance between every possible pair of points and keeps track of the minimum distance.

**Time Complexity:** `O(n²)`

**Auxiliary Space Complexity:** `O(1)`

### 2. Divide and Conquer Approach

The Divide and Conquer algorithm solves the problem by dividing the points into smaller subsets.

**Working:**

1. Sort the points according to their X-coordinates.
2. Divide the points into two halves.
3. Recursively find the closest pair in each half.
4. Calculate the minimum distance between the two halves.
5. Create a strip around the dividing line.
6. Check nearby points in the strip for a potentially closer pair.
7. Return the overall closest pair.

**Time Complexity:** `O(n log n)`

**Auxiliary Space Complexity:** `O(n)`

The implementation maintains points sorted by both X and Y coordinates to avoid repeated sorting during recursive calls.

## 📊 Algorithm Comparison

| Parameter | Brute Force | Divide and Conquer |
|---|---|---|
| Strategy | Exhaustive Search | Divide and Conquer |
| Time Complexity | O(n²) | O(n log n) |
| Auxiliary Space | O(1) | O(n) |
| Approach | Iterative | Recursive |
| Efficiency | Less efficient for large inputs | More efficient for large inputs |

## 🛠️ Technologies Used

| Category | Technology |
|---|---|
| Programming Language | C |
| GUI Framework | Windows API (Win32 API) |
| Graphics | Windows GDI |
| Mathematical Library | math.h |
| Compiler | MinGW GCC |
| IDE | Code::Blocks / VS Code |
| Operating System | Windows |

## 🚀 Getting Started

### Prerequisites

- Windows operating system
- MinGW-w64 GCC compiler
- Code::Blocks or Visual Studio Code

### 1. Clone the Repository

```bash
git clone https://github.com/YOUR_USERNAME/Closest-Pair-of-Points-Finder.git
```

### 2. Navigate to the Project Directory

```bash
cd Closest-Pair-of-Points-Finder
```

### 3. Compile the Application

```bash
gcc -std=c11 -Wall -Wextra -O2 closest_pair_gui.c -o closest_pair.exe -mwindows -lgdi32 -luser32 -lm
```

### 4. Run the Application

```bash
closest_pair.exe
```

**Note:** Replace `closest_pair_gui.c` with the actual source filename if you have not renamed it.

## 🧪 Testing

The project includes test cases for verifying algorithm correctness.

The test cases cover:

- Minimum input size (2 points)
- Duplicate points
- Collinear points
- Points with identical X-coordinates
- Points with identical Y-coordinates
- Randomized consistency testing

Compile the testing version using:

```bash
gcc -std=c11 -Wall -Wextra -O2 -DCLOSEST_PAIR_TEST closest_pair_gui.c -o closest_pair_tests.exe -lm
```

Run the tests:

```bash
closest_pair_tests.exe
```

The test program compares the results of both algorithms to verify correctness.

## 🖥️ Application Preview

<img width="1081" height="603" alt="image" src="https://github.com/user-attachments/assets/b7bf1fd8-2de9-433e-be53-c44a97978dfc" />


## 🔮 Future Improvements

- Visualize the recursive Divide and Conquer process.
- Display the dividing line and strip during execution.
- Add the number of comparisons and recursive calls.
- Implement performance graphs for different input sizes.
- Add adjustable animation speed.
- Support larger datasets and additional visualization options.

## 🎯 Learning Outcomes

Through this project, we aim to understand:

- Divide and Conquer design strategy.
- Recursive algorithm implementation.
- Brute Force problem-solving techniques.
- Computational Geometry fundamentals.
- Euclidean distance calculation.
- Time and space complexity analysis.
- Performance comparison between algorithms.
- C programming and Windows GUI development.

## 👨‍💻 Contributors

Developed as a **Design and Analysis of Algorithms (DAA) Mini Project** for SY B.Tech Computer Science and Engineering.

---

⭐ If you find this project interesting, consider giving the repository a star!

**Built with C | Powered by Divide and Conquer | Focused on Algorithmic Efficiency**
