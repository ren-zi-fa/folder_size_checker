# Top Level Folder Size Checker

A simple C program for **displaying the direct file size** of each top-level folder in a given path on Windows.  
Useful for quickly checking the size of multiple project folders without counting files inside their subfolders.

## Features

- Calculates the **total size of files directly inside** each top-level folder (does not recurse into subfolders).
- Can **exclude specific folders** from listing (default: `node_modules` and `.git`).
- Displays size in both bytes and **human-readable** format (KB, MB, GB).
- Supports **Unicode paths** on Windows.

## usage

```sh
git clone https://github.com/ren-zi-fa/folder_size_checker.git
cd folder_size_checker
check.exe  <folder_path>
```

## example

```sh
check.exe E:\my-project
```

## picture

[Screenshot](https://github.com/ren-zi-fa/folder_size_checker/blob/main/image.png)
