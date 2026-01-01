#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H
#include <chrono>
#include <random>
#include <unordered_map>
#include <glm/glm.hpp>
#include <string>

namespace Glb {

    // 计时器类,用于性能分析和FPS计算
    class Timer {
    public:
        static Timer& getInstance() {
            static Timer instance;
            return instance;
        }

    private:
        Timer() {

        };

        Timer(const Timer&) = delete;
        Timer& operator=(const Timer&) = delete;

        std::chrono::system_clock::time_point fpsLastTime;  // FPS计算的上一帧时间
        std::chrono::system_clock::time_point fpsNow;       // FPS计算的当前帧时间

        std::chrono::system_clock::time_point lastTime;     // 性能分析的上一时间点
        std::chrono::system_clock::time_point now;          // 性能分析的当前时间点

        std::unordered_map<std::string, unsigned long long int> record;  // 记录各阶段耗时

    public:
        // 检查记录是否为空
        bool empty() {
            return record.empty();
        }

        // 清空记录
        void clear() {
            record.clear();
        }

        // 开始计时
        void start() {
            lastTime = std::chrono::system_clock::now();
        } 

        // 计算FPS
        void timeFPS() {
            fpsLastTime = fpsNow;
            fpsNow = std::chrono::system_clock::now();
        }

        // 获取FPS值
        std::string getFPS() {
            auto dur = fpsNow - fpsLastTime;
            float dt = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
            if (dt >= 1000) {
                return "0.00";
            }
            else if (dt <= 1) {
                return "1000.00";
            }
            return std::to_string(1000 / dt).substr(0, 5);
        }

        // 记录某个阶段的耗时
        void recordTime(std::string str) {
            now = std::chrono::system_clock::now();
            auto dur = now - lastTime;
            lastTime = now;
            auto it = record.find(str);
            if (it != record.end()) {
                it->second = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
            }
            else {
                record[str] = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
            }
        }

        // 获取当前性能统计信息
        std::string currentStatus() {
            std::string str;
            int total_time = 0;
            for (const auto& timing : record) {
                total_time += timing.second;
            }

            for (const auto& timing : record) {
                float percentage = static_cast<float>(timing.second) / total_time * 100;
                str += timing.first + ": " + std::to_string(percentage).substr(0, 5) + "%% \n";
            }

            return str;
        }
    };

    // 随机数生成器类
    class RandomGenerator {
    private:
        std::random_device dev;
    public:
        // 生成指定范围内的随机浮点数
        float GetUniformRandom(float min = 0.0f, float max = 1.0f) {
            std::mt19937 rng(dev());
            std::uniform_real_distribution<float> dist(min, max);
            return dist(rng);
        }
    };

}

#endif




