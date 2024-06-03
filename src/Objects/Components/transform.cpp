#pragma once

#include <Unified-Engine/Objects/Components/transform.h>

namespace UnifiedEngine
{
    /// @brief Sets the rotation using a quaternion
    /// @param rotation A quaternion
    void Transform::SetRotation(glm::quat rotation) {
        this->Quaternion = rotation;
        NormalizeQuaternion(this->Quaternion);
        this->Rotation = glm::eulerAngles(this->Quaternion);
        this->Rotation.x = glm::degrees(this->Rotation.x);
        this->Rotation.y = glm::degrees(this->Rotation.y);
        this->Rotation.z = glm::degrees(this->Rotation.z);
        CalculateNewDirectionVectors();
    }
    /// @brief Rotates the transform using euler angles
    /// @param rotation Euler angles in degrees
    void Transform::SetRotation(glm::vec3 rotation) {
        this->Rotation = rotation;
        NormalizeAngles(this->Rotation);
        this->Quaternion = glm::quat(glm::vec3(glm::radians(this->Rotation.x), glm::radians(this->Rotation.y), glm::radians(this->Rotation.z)));
        CalculateNewDirectionVectors();
    }
    /// @brief Rotates the transform using a quaternion
    /// @param rotation A quaternion
    void Transform::Rotate(glm::quat rotation) {
        NormalizeQuaternion(rotation);
        this->Quaternion = rotation * this->Quaternion; //* glm::inverse(rotation);
        NormalizeQuaternion(this->Quaternion);
        this->Rotation = glm::eulerAngles(this->Quaternion);
        this->Rotation.x = glm::degrees(this->Rotation.x);
        this->Rotation.y = glm::degrees(this->Rotation.y);
        this->Rotation.z = glm::degrees(this->Rotation.z);
        CalculateNewDirectionVectors();
    }
    /// @brief Rotates the transform using euler angles
    /// @param rotation A vector made of euler angles in degrees
    void Transform::Rotate(glm::vec3 rotation) {
        this->Rotation += rotation;
        NormalizeAngles(this->Rotation);
        this->Quaternion = glm::quat(glm::vec3(glm::radians(this->Rotation.x), glm::radians(this->Rotation.y), glm::radians(this->Rotation.z)));
        NormalizeQuaternion(this->Quaternion);
        CalculateNewDirectionVectors();
    }
    /// @brief Rotates the object so that it would look at the given position
    /// @param position The position to look at
    void Transform::LookAt(glm::vec3 position) {
        glm::vec3 relativevec = position - this->Position;
        glm::vec3 axis = glm::cross(this->front, relativevec);
        Normalize3DVector(axis);
        float angle = glm::degrees(std::acos(glm::dot(this->front, relativevec) / (GetMagnitude(this->front) * GetMagnitude(relativevec))));
        if (angle != 0 && angle == angle) {
            Transform::Rotate(GetQuaternionFromPolar(angle, axis));
            CalculateNewDirectionVectors();
        }
    }
    /// @brief Rotates the object by the given step size so that it would rotate towards the given position
    /// @param position The position to look at
    /// @param stepsize The percentage of the rotation that should be completed on this step
    void Transform::SLerp(glm::vec3 position, float stepsize) {
        glm::vec3 relativevec = position - this->Position;
        glm::vec3 axis = glm::cross(this->front, relativevec);
        Normalize3DVector(axis);
        float angle = glm::degrees(std::acos(glm::dot(this->front, relativevec) / (GetMagnitude(this->front) * GetMagnitude(relativevec)))) * stepsize;
        if (angle != 0 && angle == angle) {
            Transform::Rotate(GetQuaternionFromPolar(angle, axis));
            CalculateNewDirectionVectors();
        }
    }
    /// @brief Moves the transform using the direction vectors
    /// @param movement How much of the right, up and front vectors the object will be moved by
    void Transform::Move(glm::vec3 movement) {
        glm::vec3 movementvec;
        movementvec += movement.x * this->right;
        movementvec += movement.y * this->up;
        movementvec += movement.z * this->front;
        this->Position += movementvec;
    }
    /// @brief Updates the new direction vectors
    void Transform::CalculateNewDirectionVectors() {
        this->front = this->Quaternion * glm::vec3(0, 0, 1);
        this->up = this->Quaternion * glm::vec3(0, 1, 0);
        this->right = this->Quaternion * glm::vec3(-1, 0, 0);
    }
} // namespace UnifiedEngine
