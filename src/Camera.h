#pragma once

struct Camera {

	glm::vec3 position{ 0,10,30 };
	glm::vec3 target{ 0,10,0 };
	float fov = 45;
	float aspect = 16.f / 9.f;
	float znear = .1f;
	float zfar = 1000;
	float speed = 10;
	float sensitivity = .1f;

	glm::mat4 view, proj;
	glm::vec3 front, right, up;
	float yaw, pitch;

	Camera() {
		front = glm::normalize(target - position);
		right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
		up = glm::normalize(glm::cross(right, front));
		yaw = glm::degrees(atan2(front.z, front.x));
		pitch = glm::degrees(asin(front.y));
	}

	void Update(GLFWwindow* window, float dt, glm::ivec2 delta_pos) {

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			position += front * speed * dt;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			position -= front * speed * dt;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			position += right * speed * dt;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			position -= right * speed * dt;
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			position += up * speed * dt;
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			position -= up * speed * dt;

		if (delta_pos != glm::ivec2(0) &&
			glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
			yaw += delta_pos.x * sensitivity;
			pitch -= delta_pos.y * sensitivity;
			pitch = glm::clamp(pitch, -89.f, 89.f);
			front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
			front.y = sin(glm::radians(pitch));
			front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
			right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
			up = glm::normalize(glm::cross(right, front));
		}

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		if (height != 0)aspect = width / (float)height;

		target = position + front;
		view = glm::lookAt(position, target, glm::vec3(0, 1, 0));
		proj = glm::perspective(glm::radians(fov), aspect, znear, zfar);
	}
};