#include "MMDModel.h"

MMDModel::MMDModel(const fs::path& path) {

	std::vector<byte> data = File::ReadAllBytes(path);
	BinaryReader br(data.data());

	br.Seek(9);

	struct Header {
		char encode;        // 0:UTF-16 1:UTF-8
		char additional_uv; // 0-4
		char vertex;        // 1,2,4
		char texture;       // 1,2,4
		char material;      // 1,2,4
		char bone;          // 1,2,4
		char morph;         // 1,2,4
		char rigidbody;     // 1,2,4
	};
	Header header;
	br.Read(&header, 8);

	// model info
	for (int i = 0; i < 4; i++) {
		int size = br.ReadInt();
		br.Seek(size);
	}

	// vertex
	int vertex_count = br.ReadInt();
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::ivec4 bones;
		glm::vec4 weights;
		char skinning_type;
		glm::vec3 center, r0, r1;
	};
	std::vector<Vertex> vertices(vertex_count);
	for (Vertex& vertex : vertices) {

		br.Read(&vertex.position, 12);
		vertex.position.z *= -1;
		br.Read(&vertex.normal, 12);
		vertex.normal.z *= -1;
		br.Read(&vertex.uv, 8);
		vertex.uv[1] = 1 - vertex.uv[1];
		br.Seek(header.additional_uv * 16);

		vertex.skinning_type = br.ReadChar();
		switch (vertex.skinning_type) {
		case 0: // BDEF1
			vertex.bones[0] = br.ReadSize(header.bone);
			vertex.weights[0] = 1;
			break;
		case 1: // BDEF2
		case 3: // SDEF
			for (int i = 0; i < 2; i++)
				vertex.bones[i] = br.ReadSize(header.bone);
			vertex.weights[0] = br.ReadFloat();
			vertex.weights[1] = 1 - vertex.weights[0];

			if (vertex.skinning_type == 3) {
				glm::vec3 center, r0, r1;
				br.Read(&center, 12);
				br.Read(&r0, 12);
				br.Read(&r1, 12);
				center.z *= -1;
				r0.z *= -1;
				r1.z *= -1;
				glm::vec3 rw = r0 * vertex.weights[0] + r1 * vertex.weights[1];
				r0 = center + r0 - rw;
				r1 = center + r1 - rw;
				vertex.center = center;
				vertex.r0 = (center + r0) * .5f;
				vertex.r1 = (center + r1) * .5f;
			}
			break;
		case 2: // BDEF4
			for (int i = 0; i < 4; i++)
				vertex.bones[i] = br.ReadSize(header.bone);
			br.Read(&vertex.weights, 4 * 4);
			break;
		}

		br.Seek(4); // edge
	}

	// index
	int index_count = br.ReadInt();
	std::vector<uint> indices(index_count);
	for (int i = 0; i < index_count; i++)
		indices[i] = br.ReadUSize(header.vertex);
	for (int i = 0; i < index_count / 3; i++)
		std::swap(indices[i * 3 + 0], indices[i * 3 + 1]);

	// build buffers
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof Vertex, (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof Vertex, (void*)offsetof(Vertex, normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof Vertex, (void*)offsetof(Vertex, uv));

	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 4, GL_INT, sizeof Vertex, (void*)offsetof(Vertex, bones));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, false, sizeof Vertex, (void*)offsetof(Vertex, weights));

	glEnableVertexAttribArray(5);
	glVertexAttribIPointer(5, 1, GL_INT, sizeof Vertex, (void*)offsetof(Vertex, skinning_type));

	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 3, GL_FLOAT, false, sizeof Vertex, (void*)offsetof(Vertex, center));

	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 3, GL_FLOAT, false, sizeof Vertex, (void*)offsetof(Vertex, r0));

	glEnableVertexAttribArray(8);
	glVertexAttribPointer(8, 3, GL_FLOAT, false, sizeof Vertex, (void*)offsetof(Vertex, r1));

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint), indices.data(), GL_STATIC_DRAW);

	// texture
	int texture_count = br.ReadInt();
	fs::path dir = path.parent_path();
	for (int i = 0; i < texture_count; i++) {
		int size = br.ReadInt();
		std::wstring name(size / 2, 0);
		br.Read(name.data(), size);
		textures.emplace_back(dir / name);
	}

	// material
	int material_count = br.ReadInt();
	materials.resize(material_count);
	int offset = 0;
	for (Material& material : materials) {

		// name
		for (int i = 0; i < 2; i++) {
			int size = br.ReadInt();
			br.Seek(size);
		}

		br.Seek(16 + 12 + 4 + 12); // material
		material.flag = br.ReadChar();
		br.Seek(16 + 4); // edge

		material.diffuse_texture = br.ReadSize(header.texture);
		br.Seek(header.texture + 1); // sphere texture and mode
		char toon_flag = br.ReadChar();
		if (toon_flag == 0)br.Seek(header.texture);
		else br.Seek(1);

		// memo
		int size = br.ReadInt();
		br.Seek(size);

		material.index_count = br.ReadInt();
		material.index_offset = offset;
		offset += material.index_count;
	}

	// bone
	int bone_count = br.ReadInt();
	bones.resize(bone_count);
	FinalTransform.resize(bone_count, glm::mat4(1));
	BoneRotation.resize(bone_count, glm::quat(1, 0, 0, 0));
	glGenBuffers(1, &ubo);
	glGenBuffers(1, &sdef_ubo);
	for (Bone& bone : bones) {

		int size = br.ReadInt();
		bone.name.resize(size / 2);
		br.Read(bone.name.data(), size);
		size = br.ReadInt();
		br.Seek(size); // english

		br.Read(&bone.position, 12);
		bone.position.z *= -1;
		bone.InverseBindPose = glm::translate(glm::mat4(1), -bone.position);

		int parent = br.ReadSize(header.bone);
		if (parent != -1) {
			bone.parent = &bones[parent];
			bone.FromParent = bone.position - bone.parent->position;
			bone.parent->children.push_back(&bone);
		}
		else
			bone.FromParent = bone.position;

		br.Seek(4); // �ό`�K�w

		bone.flag = (BoneFlag)br.ReadShort();

		if (!(bone.flag & connection)) {
			br.Read(&bone.offset, 12);
			bone.offset.z *= -1;
		}
		else {
			int child = br.ReadSize(header.bone);
			if (child != -1)bone.child = &bones[child];
		}

		if (bone.flag & rotate_append || bone.flag & translate_append) {
			append_bones.push_back(&bone);
			int append_parent = br.ReadSize(header.bone);
			bone.append_parent = &bones[append_parent];
			bone.append_weight = br.ReadFloat();
		}

		if (bone.flag & axis_fix)br.Seek(12);
		if (bone.flag & local_axis)br.Seek(12 + 12);
		if (bone.flag & external_parent_deform)br.Seek(4);

		if (bone.flag & ik) {
			ik_bones.push_back(&bone);

			int target_bone = br.ReadSize(header.bone);
			bone.target_bone = &bones[target_bone];
			bone.loop_count = br.ReadInt();
			bone.angle_limit = br.ReadFloat();
			int link_count = br.ReadInt();
			bone.links.resize(link_count);

			for (Bone::IKLink& link : bone.links) {
				int link_bone = br.ReadSize(header.bone);
				link.link_bone = &bones[link_bone];
				link.link_bone->is_link_bone = true;
				link.angle_lock = br.ReadChar();

				if (link.angle_lock) {
					br.Read(&link.lower_limit, 12 + 12);
					link.lower_limit.x *= -1;
					link.lower_limit.y *= -1;
					link.upper_limit.x *= -1;
					link.upper_limit.y *= -1;
					std::swap(link.lower_limit.x, link.upper_limit.x);
					std::swap(link.lower_limit.y, link.upper_limit.y);
				}
			}
		}
	}
	for (Bone& bone : bones)
		if (bone.child)
			bone.offset = bone.child->position - bone.position;

	// morph
	int morph_count = br.ReadInt();
	morphs.resize(morph_count);
	for (Morph& morph : morphs) {

		int size = br.ReadInt();
		morph.name.resize(size / 2);
		br.Read(morph.name.data(), size);
		size = br.ReadInt();
		br.Seek(size); // engilish

		br.Seek(1); // ����p�l��
		morph.type = br.ReadChar();
		morph.offset_count = br.ReadInt();

		switch (morph.type) {
		case 0: // group
			morph.group_morphs.resize(morph.offset_count);
			for (Morph::GroupMorph& group_morph : morph.group_morphs) {
				group_morph.morph = br.ReadSize(header.morph);
				group_morph.weight = br.ReadFloat();
			}
			break;
		case 1: // vertex
			morph.vertex_morphs.resize(morph.offset_count);
			for (Morph::VertexMorph& vertex_morph : morph.vertex_morphs) {
				vertex_morph.vertex = br.ReadSize(header.vertex);
				br.Read(&vertex_morph.offset, 12);
				vertex_morph.offset.z *= -1;
			}
			break;
		case 2: // bone
			br.Seek(morph.offset_count * (header.bone + 12 + 16));
			break;
		case 3: // uv
		case 4: // additional uv 1
		case 5: // additional uv 2
		case 6: // additional uv 3
		case 7: // additional uv 4
			br.Seek(morph.offset_count * (header.vertex + 16));
			break;
		case 8: // material
			br.Seek(morph.offset_count * (header.material + 1 + 16 + 12 + 4 + 12 + 16 + 4 + 16 + 16 + 16));
			break;
		}
	}
	morph_pos.resize(vertex_count);
	glBindVertexArray(vao);
	glGenBuffers(1, &morph_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, morph_vbo);
	glEnableVertexAttribArray(9);
	glVertexAttribPointer(9, 3, GL_FLOAT, false, 0, 0);

	// display
	int display_count = br.ReadInt();
	for (int i = 0; i < display_count; i++) {
		for (int i = 0; i < 2; i++) {
			int size = br.ReadInt();
			br.Seek(size);
		}
		br.Seek(1);
		int element_count = br.ReadInt();
		for (int j = 0; j < element_count; j++) {
			char type = br.ReadChar();
			br.Seek(type == 0 ? header.bone : header.morph);
		}
	}

	// physics
	auto configuration = new btDefaultCollisionConfiguration;
	auto dispatcher = new btCollisionDispatcher(configuration);
	auto overlappingPairCache = new btDbvtBroadphase;
	auto solver = new btSequentialImpulseConstraintSolver;
	world = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, configuration);
	world->setGravity(btVector3(0, -10 * 10, 0));
	DebugDrawer* drawer = new DebugDrawer();
	world->setDebugDrawer(drawer);

	// rigidbody
	int rigidbody_count = br.ReadInt();
	bodies.resize(rigidbody_count);
	for (RigidBody& body : bodies) {

		// name
		for (int i = 0; i < 2; i++) {
			int size = br.ReadInt();
			br.Seek(size);
		}

		int bone = br.ReadSize(header.bone);
		if (bone != -1)body.bone = &bones[bone];
		br.Read(&body.group, 1 + 2 + 1 + 12 + 12 + 12 + 4 + 4 + 4 + 4 + 4 + 1);

		body.position.z *= -1;
		body.rotation.x *= -1;
		body.rotation.y *= -1;

		btCollisionShape* shape = NULL;
		switch (body.shape) {
		case 0:
			shape = new btSphereShape(body.size.x);
			break;
		case 1:
			shape = new btBoxShape(btVector3(body.size.x, body.size.y, body.size.z));
			break;
		case 2:
			shape = new btCapsuleShape(body.size.x, body.size.y);
			break;
		}

		float mass = 0;
		btVector3 localInertia(0, 0, 0);
		if (body.type != 0) {
			mass = body.mass;
			shape->calculateLocalInertia(mass, localInertia);
		}

		glm::mat4 transform = glm::eulerAngleYXZ(
			body.rotation.y,
			body.rotation.x,
			body.rotation.z
		);
		transform[3] = glm::vec4(body.position, 1);

		btTransform trans;
		trans.setFromOpenGLMatrix((float*)&transform);
		auto motion = new btDefaultMotionState(trans);

		btRigidBody::btRigidBodyConstructionInfo info(mass, motion, shape, localInertia);
		info.m_friction = body.friction;
		info.m_restitution = body.restitution;
		info.m_linearDamping = body.linear_damping;
		info.m_angularDamping = body.angular_damping;
		body.btBody = new btRigidBody(info);

		if (body.type == 0) {
			body.btBody->setCollisionFlags(body.btBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		}
		body.btBody->setActivationState(DISABLE_DEACTIVATION);

		world->addRigidBody(body.btBody, 1 << body.group, body.mask);

		// bone and rigid body positions are a little different
		if (body.bone) {
			body.fromBone = body.bone->InverseBindPose * transform;
			body.fromBody = glm::inverse(body.fromBone);
		}
	}

	// joint
	int joint_count = br.ReadInt();
	for (int i = 0; i < joint_count; i++) {

		struct Joint {
			int bodyA;
			int bodyB;
			glm::vec3 position;
			glm::vec3 rotation; // radian
			glm::vec3 linear_lower_limit;
			glm::vec3 linear_upper_limit;
			glm::vec3 angular_lower_limit;
			glm::vec3 angular_upper_limit;
			glm::vec3 linear_spring_constant;
			glm::vec3 angular_spring_constant;
		};
		Joint joint;

		// name
		for (int i = 0; i < 2; i++) {
			int size = br.ReadInt();
			br.Seek(size);
		}

		br.Seek(1); // type
		joint.bodyA = br.ReadSize(header.rigidbody);
		joint.bodyB = br.ReadSize(header.rigidbody);
		br.Read(&joint.position, 12 * 8);

		joint.position.z *= -1;
		joint.rotation.x *= -1;
		joint.rotation.y *= -1;

		if (joint.bodyA == joint.bodyB)continue;
		RigidBody& bodyA = bodies[joint.bodyA];
		RigidBody& bodyB = bodies[joint.bodyB];

		glm::mat4 transform = glm::eulerAngleYXZ(
			joint.rotation.y,
			joint.rotation.x,
			joint.rotation.z
		);
		transform[3] = glm::vec4(joint.position, 1);

		btTransform trans;
		trans.setFromOpenGLMatrix((float*)&transform);

		btTransform frameInA = bodyA.btBody->getWorldTransform();
		frameInA = frameInA.inverse() * trans;
		btTransform frameInB = bodyB.btBody->getWorldTransform();
		frameInB = frameInB.inverse() * trans;

		auto constraint = new btGeneric6DofSpringConstraint(*bodyA.btBody, *bodyB.btBody, frameInA, frameInB, true);
		world->addConstraint(constraint);

		constraint->setLinearLowerLimit(btVector3(
			joint.linear_lower_limit.x,
			joint.linear_lower_limit.y,
			joint.linear_lower_limit.z
		));
		constraint->setLinearUpperLimit(btVector3(
			joint.linear_upper_limit.x,
			joint.linear_upper_limit.y,
			joint.linear_upper_limit.z
		));
		constraint->setAngularLowerLimit(btVector3(
			joint.angular_lower_limit.x,
			joint.angular_lower_limit.y,
			joint.angular_lower_limit.z
		));
		constraint->setAngularUpperLimit(btVector3(
			joint.angular_upper_limit.x,
			joint.angular_upper_limit.y,
			joint.angular_upper_limit.z
		));

		// ���̒e����L����
		if (joint.linear_spring_constant.x != 0) {
			constraint->enableSpring(0, true);
			constraint->setStiffness(0, joint.linear_spring_constant.x);
		}
		if (joint.linear_spring_constant.y != 0) {
			constraint->enableSpring(1, true);
			constraint->setStiffness(1, joint.linear_spring_constant.y);
		}
		if (joint.linear_spring_constant.z != 0) {
			constraint->enableSpring(2, true);
			constraint->setStiffness(2, joint.linear_spring_constant.z);
		}
		if (joint.angular_spring_constant.x != 0) {
			constraint->enableSpring(3, true);
			constraint->setStiffness(3, joint.angular_spring_constant.x);
		}
		if (joint.angular_spring_constant.y != 0) {
			constraint->enableSpring(4, true);
			constraint->setStiffness(4, joint.angular_spring_constant.y);
		}
		if (joint.angular_spring_constant.z != 0) {
			constraint->enableSpring(5, true);
			constraint->setStiffness(5, joint.angular_spring_constant.z);
		}
	}
}

void MMDModel::Update(float dt) {

	if (!anim)return;

	anim_frame += dt * 30;
	if (anim_frame > anim->max_frame) {
		anim_frame = fmodf(anim_frame, (float)anim->max_frame);
		if (music) {
			music->stop();
			music->play();
		}
	}

	for (Bone& bone : bones) {

		bone.Translation = glm::vec3(0);
		bone.Rotation = glm::quat(1, 0, 0, 0);

		if (anim->bone_map.count(bone.name) > 0) {

			std::vector<VMDAnimation::BoneFrame>& bone_frames = anim->bone_map[bone.name];

			int i = 0;
			for (; i < bone_frames.size(); i++)
				if (bone_frames[i].frame > anim_frame)break;

			VMDAnimation::BoneFrame* bf0;
			VMDAnimation::BoneFrame* bf1;
			float t;

			if (i == 0 || i == bone_frames.size()) {
				bf0 = &bone_frames.back();
				bf1 = &bone_frames.front();
				int rest_frame = anim->max_frame - bf0->frame;
				if (i == 0) t = (rest_frame + anim_frame) / (rest_frame + bf1->frame);
				else t = (anim_frame - bf0->frame) / (rest_frame + bf1->frame);
			}
			else {
				bf0 = &bone_frames[i - 1];
				bf1 = &bone_frames[i];
				t = (anim_frame - bf0->frame) / (bf1->frame - bf0->frame);
			}

			bone.Translation = glm::mix(bf0->translation, bf1->translation, t);
			bone.Rotation = glm::slerp(bf0->rotation, bf1->rotation, t);
		}

		// CCD-IK�̊p�x�����̂��߂ɑ���O��90�x��]������
		if (bone.name == L"�E��" || bone.name == L"����")
			bone.Rotation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1, 0, 0));

		bone.UpdateLocalTransform();
		bone.UpdateGlobalTransform(false);
	}

	ProcessIK();

	// append parent
	for (Bone* bone : append_bones) {
		if (bone->flag & BoneFlag::rotate_append)
			bone->Rotation = bone->append_parent->Rotation;
		if (bone->flag & BoneFlag::translate_append)
			bone->Translation = bone->append_parent->Translation;
		bone->UpdateLocalTransform();
		bone->UpdateGlobalTransform(true);
	}

	// physics
	if (Globals::Physics) {
		for (RigidBody& body : bodies) {
			// do kinematic
			if (body.type != 0)continue;
			if (!body.bone)continue;

			btTransform transform;
			glm::mat4 mat = body.bone->GlobalTransform * body.fromBone;
			transform.setFromOpenGLMatrix((float*)&mat);
			body.btBody->getMotionState()->setWorldTransform(transform);
		}

		world->stepSimulation(dt);

		for (RigidBody& body : bodies) {
			// do dynamic
			if (body.type == 0)continue;
			if (!body.bone)continue;

			btTransform transform;
			body.btBody->getMotionState()->getWorldTransform(transform);
			glm::mat4 mat;
			transform.getOpenGLMatrix((float*)&mat);
			body.bone->GlobalTransform = mat * body.fromBody;
		}
	}

	for (int i = 0; i < bones.size(); i++)
		FinalTransform[i] = bones[i].GlobalTransform * bones[i].InverseBindPose;

	// morph
	std::fill(morph_pos.begin(), morph_pos.end(), glm::vec3(0));
	for (Morph& morph : morphs) {
		if (anim->morph_map.count(morph.name) > 0) {
			std::vector<VMDAnimation::MorphFrame>& morph_frames = anim->morph_map[morph.name];

			int i = 0;
			for (; i < morph_frames.size(); i++)
				if (morph_frames[i].frame > anim_frame)break;

			float weight;
			if (i == 0)
				weight = morph_frames.front().weight;
			else if (i == morph_frames.size())
				weight = morph_frames.back().weight;
			else {
				VMDAnimation::MorphFrame& a = morph_frames[i - 1];
				VMDAnimation::MorphFrame& b = morph_frames[i];
				float t = (anim_frame - a.frame) / (b.frame - a.frame);
				weight = glm::mix(a.weight, b.weight, t);
			}

			ProcessMorph(morph, weight);
		}
	}
}

void MMDModel::Draw(Shader* shader, Camera* camera) {

	shader->Use();

	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, FinalTransform.size() * sizeof(glm::mat4), FinalTransform.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

	glBindBuffer(GL_ARRAY_BUFFER, morph_vbo);
	glBufferData(GL_ARRAY_BUFFER, morph_pos.size() * sizeof(glm::vec3), morph_pos.data(), GL_DYNAMIC_DRAW);

	glm::mat4 mvp = camera->proj * camera->view * transform;
	glUniformMatrix4fv(shader->uniforms["mvp"], 1, false, (float*)&mvp);

	// sdef
	for (int i = 0; i < bones.size(); i++) {
		BoneRotation[i] = glm::quat_cast(bones[i].GlobalTransform);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, sdef_ubo);
	glBufferData(GL_UNIFORM_BUFFER, BoneRotation.size() * sizeof(glm::quat), BoneRotation.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, sdef_ubo);

	glBindVertexArray(vao);

	for (Material& material : materials) {

		if (material.diffuse_texture != -1)
			glBindTexture(GL_TEXTURE_2D, textures[material.diffuse_texture].id);
		else
			glBindTexture(GL_TEXTURE_2D, 0);

		if (material.flag & 0x01)glDisable(GL_CULL_FACE);
		else glEnable(GL_CULL_FACE);

		glDrawElements(GL_TRIANGLES, material.index_count, GL_UNSIGNED_INT, (void*)(material.index_offset * sizeof(uint)));
	}

	glEnable(GL_CULL_FACE);

	glUseProgram(0);
	glm::mat4 mv = camera->view * transform;
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf((float*)&mv);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf((float*)&camera->proj);

	if (Globals::IKBone)DrawIKBone(camera);
	if (Globals::RigidBody)world->debugDrawWorld();
	if (Globals::AABB) DrawAABB(camera);
}

void MMDModel::SetAnimation(VMDAnimation* anim, sf::Music* music) {
	this->anim = anim;
	anim_frame = 0;
	this->music = music;
}

void MMDModel::ProcessIK() {

	for (Bone* ik_bone : ik_bones) {
		Bone* target_bone = ik_bone->target_bone;
		glm::vec3 ik_pos = ik_bone->GlobalTransform[3];

		for (int i = 0; i < ik_bone->loop_count; i++) {
			glm::vec3 target_pos = target_bone->GlobalTransform[3];
			float distance = glm::distance(ik_pos, target_pos);
			if (distance < 0.01)break;

			for (Bone::IKLink& link : ik_bone->links) {
				Bone* link_bone = link.link_bone;
				glm::vec3 target_pos = target_bone->GlobalTransform[3];
				glm::mat4 inv_link = glm::inverse(link_bone->GlobalTransform);

				glm::vec3 local_ik_pos = inv_link * glm::vec4(ik_pos, 1);
				glm::vec3 local_target_pos = inv_link * glm::vec4(target_pos, 1);

				glm::vec3 a = glm::normalize(local_ik_pos);
				glm::vec3 b = glm::normalize(local_target_pos);

				float dot = glm::dot(a, b);
				dot = glm::clamp(dot, -1.f, 1.f);

				float angle = acos(dot);
				angle = glm::clamp(angle, 0.f, ik_bone->angle_limit);
				if (angle < 0.001)continue;

				glm::vec3 axis = glm::cross(b, a);
				axis = glm::normalize(axis);

				glm::quat ik_rot = glm::angleAxis(angle, axis);
				link_bone->Rotation = link_bone->Rotation * ik_rot;

				if (link.angle_lock) {
					glm::vec3 euler = glm::eulerAngles(link_bone->Rotation);
					euler = glm::clamp(euler, link.lower_limit, link.upper_limit);
					link_bone->Rotation = glm::quat(euler);
				}

				link_bone->UpdateLocalTransform();
				link_bone->UpdateGlobalTransform(true);
			}
		}
	}
}

void MMDModel::ProcessMorph(Morph& morph, float weight) {
	switch (morph.type) {
	case 0: // group
		for (Morph::GroupMorph& group_morph : morph.group_morphs)
			ProcessMorph(morphs[group_morph.morph], group_morph.weight * weight);
		break;
	case 1: // vertex
		for (Morph::VertexMorph& vertex_morph : morph.vertex_morphs)
			morph_pos[vertex_morph.vertex] += vertex_morph.offset * weight;
		break;
	}
}

void MMDModel::DrawIKBone(Camera* camera) {

	glLineWidth(4);
	glDepthFunc(GL_ALWAYS);
	glBegin(GL_LINES);
	for (Bone& bone : bones) {

		if (bone.flag & BoneFlag::ik)glColor3f(1, 1, 0);
		else if (bone.is_link_bone) glColor3f(1, 0, 0);
		else continue;

		glm::vec3 pos = bone.GlobalTransform[3];
		glm::vec3 off = bone.GlobalTransform * glm::vec4(bone.offset, 1);
		glVertex3fv((float*)&pos);
		glVertex3fv((float*)&off);
	}
	glEnd();

	glDepthFunc(GL_LESS);
	glLineWidth(1);
}

void MMDModel::DrawAABB(Camera* camera) {

	aabb.max = glm::vec3(std::numeric_limits<float>::lowest());
	aabb.min = glm::vec3(std::numeric_limits<float>::max());
	for (Bone& bone : bones) {
		if (!(bone.flag & BoneFlag::display))continue;
		if (!bone.parent)continue;

		aabb.max = glm::max(glm::vec3(bone.GlobalTransform[3]), aabb.max);
		aabb.min = glm::min(glm::vec3(bone.GlobalTransform[3]), aabb.min);
	}

	glColor3f(0, 1, 0);
	glLineWidth(4);

	glm::vec3& max = aabb.max;
	glm::vec3& min = aabb.min;

	glBegin(GL_LINE_LOOP);
	glVertex3f(max.x, max.y, max.z);
	glVertex3f(min.x, max.y, max.z);
	glVertex3f(min.x, min.y, max.z);
	glVertex3f(max.x, min.y, max.z);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(max.x, max.y, min.z);
	glVertex3f(min.x, max.y, min.z);
	glVertex3f(min.x, min.y, min.z);
	glVertex3f(max.x, min.y, min.z);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(max.x, max.y, max.z);
	glVertex3f(max.x, max.y, min.z);

	glVertex3f(max.x, min.y, max.z);
	glVertex3f(max.x, min.y, min.z);

	glVertex3f(min.x, max.y, max.z);
	glVertex3f(min.x, max.y, min.z);

	glVertex3f(min.x, min.y, max.z);
	glVertex3f(min.x, min.y, min.z);
	glEnd();

	glLineWidth(1);
}