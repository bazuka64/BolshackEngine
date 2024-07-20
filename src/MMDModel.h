#pragma once

struct MMDModel {

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

	struct Vertex {
		glm::vec3 position;
		glm::vec2 uv;
		glm::ivec4 bones;
		glm::vec4 weights;
	};

	struct Material {
		int diffuse_texture;
		int index_count;
		int index_offset;
		char flag; // 0x01:両面描画
	};

	enum BoneFlag : ushort {
		connection = 0x0001,
		rotatable = 0x0002,
		translatable = 0x0004,
		display = 0x0008,
		operatable = 0x0010,
		ik = 0x0020,
		local_append = 0x0080,
		rotate_append = 0x0100,
		translate_append = 0x0200,
		axis_fix = 0x0400,
		local_axis = 0x0800,
		physics_after_deform = 0x1000,
		external_parent_deform = 0x2000,
	};

	struct Bone {
		std::wstring name;
		glm::vec3 position; // initial
		Bone* parent;
		std::vector<Bone*> children;
		BoneFlag flag;

		// const
		glm::mat4 InverseBindPose;
		glm::vec3 FromParent;

		// dynamic
		glm::mat4 GlobalTransform;
		glm::mat4 LocalTransform;
		glm::vec3 Translation;
		glm::quat Rotation;

		// ik
		Bone* target_bone;
		int loop_count;
		float angle_limit;
		struct IKLink {
			Bone* link_bone;
			char angle_lock;
			glm::vec3 lower_limit;
			glm::vec3 upper_limit;
		};
		std::vector<IKLink> links;

		// append parent
		Bone* append_parent;
		float append_weight;

		// debug
		glm::vec3 offset;
		Bone* child;
		bool is_link_bone;

		void UpdateGlobalTransform(bool update_children) {
			if (parent)GlobalTransform = parent->GlobalTransform * LocalTransform;
			else GlobalTransform = LocalTransform;

			if (update_children)
				for (Bone* child : children)
					child->UpdateGlobalTransform(true);
		}

		void UpdateLocalTransform() {
			glm::mat4 transMat = glm::translate(glm::mat4(1), Translation + FromParent);
			glm::mat4 rotMat = glm::mat4_cast(Rotation);
			LocalTransform = transMat * rotMat;
		}
	};

	struct Morph {
		std::wstring name;
		char type; // 0:グループ, 1:頂点, 2:ボーン, 3:UV, 4:追加UV1, 5:追加UV2, 6:追加UV3, 7:追加UV4, 8:材質
		int offset_count;

		struct GroupMorph {
			int morph;
			float weight;
		};
		std::vector<GroupMorph> group_morphs;

		struct VertexMorph {
			int vertex;
			glm::vec3 offset;
		};
		std::vector<VertexMorph> vertex_morphs;
	};

	GLuint vao, vbo, ibo, ubo, morph_vbo;

	std::vector<Material> materials;
	std::vector<Texture> textures;

	std::vector<Bone> bones;
	std::vector<Bone*> ik_bones;
	std::vector<Bone*> append_bones;
	std::vector<glm::mat4> FinalTransform;

	std::vector<Morph> morphs;
	std::vector<glm::vec3> morph_pos;

	VMDAnimation* anim = NULL;
	float anim_frame = 0;
	glm::mat4 world = glm::scale(glm::mat4(1), glm::vec3(20));
	glm::vec3 local_pos;
	sf::Music* music = NULL;
	AABB aabb;

	MMDModel(const fs::path& path);
	void Update(float dt);
	void Draw(Shader& shader, Camera& camera);
	void SetAnimation(VMDAnimation* anim, sf::Music* music);

private:
	void ProcessIK();
	void ProcessMorph(Morph& morph, float weight);
	void DrawIKBone(Camera& camera);
	void DrawAABB(Camera& camera);
};

MMDModel::MMDModel(const fs::path& path) {

	std::vector<byte> data = File::ReadAllBytes(path);
	BinaryReader br(data.data());

	br.Seek(9);
	Header header;
	br.Read(&header, 8);

	// model info
	for (int i = 0; i < 4; i++) {
		int size = br.ReadInt();
		br.Seek(size);
	}

	// vertex
	int vertex_count = br.ReadInt();
	std::vector<Vertex> vertices(vertex_count);
	for (Vertex& vertex : vertices) {

		br.Read(&vertex.position, 12);
		vertex.position.z *= -1;
		br.Seek(12); // normal
		br.Read(&vertex.uv, 8);
		vertex.uv[1] = 1 - vertex.uv[1];
		br.Seek(header.additional_uv * 16);

		char skinning_type = br.ReadChar();
		switch (skinning_type) {
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
			if (skinning_type == 3)br.Seek(12 + 12 + 12);
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
	glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof Vertex, (void*)offsetof(Vertex, uv));

	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 4, GL_INT, sizeof Vertex, (void*)offsetof(Vertex, bones));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof Vertex, (void*)offsetof(Vertex, weights));

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
	glGenBuffers(1, &ubo);
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

		br.Seek(4); // 変形階層

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

		br.Seek(1); // 操作パネル
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
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, false, 0, 0);

	// display
	int display_count = br.ReadInt();
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

		// CCD-IKの角度制限のために足を前に90度回転させる
		if (bone.name == L"右足" || bone.name == L"左足")
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

void MMDModel::Draw(Shader& shader, Camera& camera) {

	shader.Use();

	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, FinalTransform.size() * sizeof(glm::mat4), FinalTransform.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

	glBindBuffer(GL_ARRAY_BUFFER, morph_vbo);
	glBufferData(GL_ARRAY_BUFFER, morph_pos.size() * sizeof(glm::vec3), morph_pos.data(), GL_DYNAMIC_DRAW);

	glm::mat4 wvp = camera.proj * camera.view * world;
	glUniformMatrix4fv(shader.uniforms["wvp"], 1, false, (float*)&wvp);

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

	if (Globals::IKBone)DrawIKBone(camera);
	if (Globals::AABB) DrawAABB(camera);
}

void MMDModel::DrawIKBone(Camera& camera) {

	glUseProgram(0);

	glm::mat4 wv = camera.view * world;
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf((float*)&wv);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf((float*)&camera.proj);

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

void MMDModel::DrawAABB(Camera& camera) {

	aabb.max = glm::vec3(std::numeric_limits<float>::lowest());
	aabb.min = glm::vec3(std::numeric_limits<float>::max());
	for (Bone& bone : bones) {
		if (!(bone.flag & BoneFlag::display))continue;
		if (!bone.parent)continue;

		aabb.max = glm::max(glm::vec3(bone.GlobalTransform[3]), aabb.max);
		aabb.min = glm::min(glm::vec3(bone.GlobalTransform[3]), aabb.min);
	}

	glUseProgram(0);

	glm::mat4 wv = camera.view * world;
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf((float*)&wv);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf((float*)&camera.proj);

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

void MMDModel::SetAnimation(VMDAnimation* anim, sf::Music* music) {
	this->anim = anim;
	anim_frame = 0;
	this->music = music;
}