/**
 * Camera.cpp: 相机类实现文件
 * 实现相机的移动、旋转、缩放和矩阵计算
 */

#include "Camera.h"
#include <iostream>

namespace Glb {

    /**
     * 处理相机移动
     * @param offset 鼠标移动偏移量
     */
    void Camera::ProcessMove(glm::vec2 offset) {
        // 根据偏移量移动相机位置
        mPosition -= offset.x * mSensitiveX * mRight;  // 左右移动
        mPosition += offset.y * mSensitiveY * mUp;     // 上下移动
        UpdateView();
    }

    /**
     * 处理相机旋转
     * @param offset 鼠标移动偏移量
     */
    void Camera::ProcessRotate(glm::vec2 offset) {
        // 处理相机旋转
        // 更新偏航角,并保持在0-360度之间
        mYaw = std::fmodf(mYaw - mSensitiveYaw * offset.x, 360.0f);
        // 更新俯仰角,并限制在-89.9到89.9度之间防止万向节死锁
        mPitch = glm::clamp(mPitch + mSensitivePitch * offset.y, -89.9f, 89.9f);
        UpdateView();
    }

    /**
     * 处理相机缩放（前后移动）
     * @param offset 滚轮偏移量
     */
    void Camera::ProcessScale(float offset) {
        // 处理相机缩放(前后移动)
        mPosition += offset * mSensitiveFront * mFront;
        UpdateView();
    }

    glm::mat4 Camera::GetView() {
        // 获取观察矩阵
        return glm::lookAt(mPosition, mPosition + mFront, mUp);
    }

    glm::mat4 Camera::GetProjection() {
        // 获取投影矩阵
        return glm::perspective(glm::radians(fovyDeg), aspect, nearPlane, farPlane);
    }

    glm::vec3 Camera::GetUp() {
        // 获取相机上方向
        return mUp;
    }

    glm::vec3 Camera::GetRight() {
        // 获取相机右方向
        return mRight;
    }

    glm::vec3 Camera::GetFront() {
        // 获取相机前方向
        return mFront;
    }

    glm::vec3 Camera::GetPosition() {
        // 获取相机位置
        return mPosition;
    }

    /**
     * 更新视图矩阵
     * 根据当前偏航角和俯仰角重新计算方向向量
     */
    void Camera::UpdateView() {
        // 更新相机的方向向量
        // 计算前方向向量
        mFront.x = std::cos(glm::radians(mPitch)) * std::cos(glm::radians(mYaw));
        mFront.y = std::cos(glm::radians(mPitch)) * std::sin(glm::radians(mYaw));
        mFront.z = std::sin(glm::radians(mPitch));
        mFront = -glm::normalize(mFront);

        // 计算右方向和上方向向量
        mRight = glm::normalize(glm::cross(mFront, mWorldUp));
        mUp = glm::normalize(glm::cross(mRight, mFront));
    }
}