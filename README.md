# SeqTools

[简体中文](./README_zh.md)

## 1 Introduction

This is a high-performance biosequence manipulation tool written in c++ that provides a command line tool (CLI) to complete various operations

## 2 features

The main features of this tool are high performance and low occupancy.

This project uses LevelDB as the sequence index tool. According to the current experimental results, 1.6GB fasta sequence (about 48w bars) can be read and indexed in about 4 seconds, with an average of 12w bars per second, which is close to the theoretical upper limit of LevelDB's write performance (14w/s).

This project uses xxhash3 as a checksum calculation tool (optional) to calculate whether the sequence file has been modified to determine if the sequence index needs to be re-established

### 3 dependence

The project currently relies on the following three open source projects

| Project name | Function Description                                         | Project address                    |
| ------------ | ------------------------------------------------------------ | ---------------------------------- |
| LevelDB      | for the establishment of the index of biological sequences and access | https://github.com/google/leveldb  |
| xxHash       | Used to compute file's checksum                              | https://github.com/Cyan4973/xxHash |
| CLI11        | scaffolding for building tools                               | https://github.com/CLIUtils/CLI11  |

## 4 Instructions for use

You can clone the project locally, and then open the project using VS to regenerate the project

## 5 insufficient

This project is currently being refined and only supports indexing and reading of fasta format files