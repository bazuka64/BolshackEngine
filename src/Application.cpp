#include "Application.h"

Application::Application() {

	WindowInit();

	camera = new Camera;

	mmd_shader = new Shader("shader/mmd.vert", "shader/mmd.frag");
	sm64_shader = new Shader("shader/sm64.vert", "shader/sm64.frag");
	mario_shader = new Shader("shader/mario.vert", "shader/mario.frag");

	float distance = 7.5f * 20;

	MMDModel* meirin = new MMDModel("res/model/meirin/meirin.pmx");
	meirin->local_pos = glm::vec3(distance, 0, 0);
	meirin->transform[3] = glm::vec4(meirin->local_pos, 1);
	models.push_back(meirin);

	MMDModel* mima = new MMDModel("res/model/mima/mima.pmx");
	mima->local_pos = glm::vec3(-distance, 0, 0);
	mima->transform[3] = glm::vec4(mima->local_pos, 1);
	models.push_back(mima);

	for (const fs::directory_entry& entry : fs::directory_iterator("res/motion/dance/")) {
		const fs::path& path = entry.path();
		fs::path dance_path = path / (path.stem().string() + ".vmd");
		dances.push_back(new VMDAnimation(dance_path));

		fs::path music_path = path / (path.stem().string() + ".wav");
		sf::Music* music = new sf::Music;
		if (!music->openFromFile(music_path.string()))throw;
		musics.push_back(music);
	}

	idle = new VMDAnimation("res/motion/1.ぼんやり待ち_(490f_移動なし).vmd");
	run = new VMDAnimation("res/motion/2.走り86L_ダッシュ加速_(14f_前移動60)修正.vmd");
	for (auto model : models)
		model->SetAnimation(idle, NULL);
	
	if (fs::exists(Globals::sm64_path))
		rom = new ROM(Globals::sm64_path);
	
	if (rom) {
		level = LevelScript::parse(rom, 0x10); // castle grounds
		for (auto model : models)
			model->transform[3] = glm::vec4(level->start_pos + model->local_pos, 1);
		camera->position = level->start_pos + camera->initial_pos;
		camera->LookAt(level->start_pos + camera->initial_target);
	}
	
	if (rom)mario = new Mario;

	// DuelMasters
	// MyAnimeList
	// Yu-Gi-Oh

	btDefaultCollisionConfiguration* configuration = new btDefaultCollisionConfiguration;
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(configuration);
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase;
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, configuration);
	dynamicsWorld->setGravity(btVector3(0, -10, 0));
	DebugDrawer* drawer = new DebugDrawer();
	dynamicsWorld->setDebugDrawer(drawer);
	
	btCollisionShape* groundShape = new btBoxShape(btVector3(500, 50, 500));

	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0, -56, 0));
	float mass = 0;
	
	btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape);
	btRigidBody* body = new btRigidBody(rbInfo);
	
	dynamicsWorld->addRigidBody(body);

	for (const fs::directory_entry& entry : fs::directory_iterator("res/image/DuelMasters/")) {
		const fs::path& path = entry.path();
		Texture* texture = new Texture(path);
		float aspect = (float)texture->width / texture->height;

		btCollisionShape* colShape = new btBoxShape(btVector3(aspect * 5, 5, 1));

		btTransform startTransform;
		startTransform.setIdentity();
		float mass = 1;

		btVector3 localInertia;
		colShape->calculateLocalInertia(mass, localInertia);

		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		dynamicsWorld->addRigidBody(body);
	}
}

void Application::Run() {

	float prev_time = (float)glfwGetTime();
	glm::ivec2 prev_pos(0);
	while (!glfwWindowShouldClose(window)) {

		float time = (float)glfwGetTime();
		float dt = time - prev_time;
		prev_time = time;

		int fps = (int)(1 / dt);
		glfwSetWindowTitle(window, std::to_string(fps).c_str());

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		glm::ivec2 pos(xpos, ypos);
		glm::ivec2 delta_pos = pos - prev_pos;
		prev_pos = pos;

		glClearColor(.1f, .1f, .5f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera->Update(window, dt, delta_pos);

		if (!Globals::Paused) {
			for (MMDModel* model : models) {
				model->Update(dt);
			}
		}

		for (MMDModel* model : models)
			model->Draw(mmd_shader, camera);

		if (level) {
			Area* area = level->areas[level->start_area];
			if (area) {
				sm64_shader->Use();
				glm::mat4 vp = camera->proj * camera->view;
				glUniformMatrix4fv(sm64_shader->uniforms["mvp"], 1, false, (float*)&vp);
				area->model->Draw(sm64_shader);

				for (Object* obj : area->objects) {
					if (!level->models.contains(obj->modelID))continue;

					glm::mat4 transform = glm::translate(glm::mat4(1), obj->position);
					glm::mat4 mvp = vp * transform;
					glUniformMatrix4fv(sm64_shader->uniforms["mvp"], 1, false, (float*)&mvp);
					level->models[obj->modelID]->Draw(sm64_shader);
				}
			}
		}

		if (mario) mario->Draw(mario_shader, camera, dt);

		dynamicsWorld->stepSimulation(dt);
		dynamicsWorld->debugDrawWorld();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (ImGui::Checkbox("WireFrame", &Globals::WireFrame)) {
			if (Globals::WireFrame)glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		ImGui::Checkbox("IKBone", &Globals::IKBone);
		ImGui::Checkbox("AABB", &Globals::AABB);
		if (ImGui::Checkbox("Paused", &Globals::Paused)) {
			if (Globals::cur_music) {
				if (Globals::Paused)Globals::cur_music->pause();
				else Globals::cur_music->play();
			}
		}
		ImGui::Checkbox("Physics", &Globals::Physics);
		ImGui::Checkbox("RigidBody", &Globals::RigidBody);

		if (rom) {
			ImGui::Separator();
			for (int i = 0; i < ROM::LevelNames.size(); i++) {
				if (i <= 3 || i == 32 || i == 35 || i >= 37)continue;
				static int selected = -1;
				if (ImGui::RadioButton(std::format("{:02X} {}", i, ROM::LevelNames[i]).c_str(), &selected, i)) {
					level = LevelScript::parse(rom, i);
					for (auto model : models)
						model->transform[3] = glm::vec4(level->start_pos + model->local_pos, 1);
					camera->position = level->start_pos + camera->initial_pos;
					camera->LookAt(level->start_pos + camera->initial_target);
				}
			}
		}

		ImGui::Separator();

		if (ImGui::Button("idle")) {
			for (MMDModel* model : models)
				model->SetAnimation(idle, NULL);
			if (Globals::cur_music)Globals::cur_music->stop();
			Globals::cur_music = NULL;
		}
		for (int i = 0; i < dances.size(); i++) {
			VMDAnimation* anim = dances[i];
			sf::Music* music = musics[i];
			if (ImGui::Button(anim->filename.c_str())) {
				for (MMDModel* model : models)
					model->SetAnimation(anim, music);
				if (Globals::cur_music)Globals::cur_music->stop();
				Globals::cur_music = music;
				music->play();
			}
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

}

Application::~Application() {
	for (sf::Music* music : musics)
		delete music;
}

void Application::WindowInit() {

	const int width = 1920, height = 1080;
	glfwInit();
	glfwWindowHint(GLFW_VISIBLE, false);
	glfwWindowHint(GLFW_SAMPLES, 4);
	window = glfwCreateWindow(width, height, "", NULL, NULL);
	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	int xpos = (mode->width - width) / 2;
	int ypos = (mode->height - height) / 2;
	glfwSetWindowPos(window, xpos, ypos);
	glfwShowWindow(window);

	gladLoadGL();
	stbi_set_flip_vertically_on_load(true);
	setlocale(LC_CTYPE, "");

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Application::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void Application::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		Globals::Paused = !Globals::Paused;
		if (Globals::cur_music) {
			if (Globals::Paused)Globals::cur_music->pause();
			else Globals::cur_music->play();
		}
	}
}

void Application::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}