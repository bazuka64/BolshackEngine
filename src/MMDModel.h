#pragma once

struct MMDModel {

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

#pragma pack(1)
	struct RigidBody {
		Bone* bone;
		char group;
		ushort mask;
		char shape; // 0:sphere 1:box 2:capsule
		glm::vec3 size;
		glm::vec3 position;
		glm::vec3 rotation; // radian
		float mass;
		float linear_damping;
		float angular_damping;
		float restitution;
		float friction;
		char type; // 0:ボーン追従(static) 1:物理演算(dynamic) 2:物理演算 + Bone位置合わせ
		btRigidBody* btBody;
		glm::mat4 fromBone;
		glm::mat4 fromBody;
	};
#pragma pack()

	GLuint vao, vbo, ibo, ubo, morph_vbo, sdef_ubo;

	std::vector<Material> materials;
	std::vector<Texture> textures;

	std::vector<Bone> bones;
	std::vector<Bone*> ik_bones;
	std::vector<Bone*> append_bones;
	std::vector<glm::mat4> FinalTransform;
	std::vector<glm::quat> BoneRotation;

	std::vector<Morph> morphs;
	std::vector<glm::vec3> morph_pos;

	btDiscreteDynamicsWorld* world;
	std::vector<RigidBody> bodies;

	VMDAnimation* anim = NULL;
	float anim_frame = 0;
	glm::mat4 transform = glm::scale(glm::mat4(1), glm::vec3(20));
	glm::vec3 local_pos;
	sf::Music* music = NULL;
	AABB aabb;
	
	MMDModel(const fs::path& path);
	void Update(float dt);
	void Draw(Shader* shader, Camera* camera);
	void SetAnimation(VMDAnimation* anim, sf::Music* music);

private:
	void ProcessIK();
	void ProcessMorph(Morph& morph, float weight);
	void DrawIKBone(Camera* camera);
	void DrawAABB(Camera* camera);
};