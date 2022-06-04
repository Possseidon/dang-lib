#include <chrono>

#include "dang-example/global.h"
#include "dang-gl/Image/BorderedImage.h"
#include "dang-glfw/GLFW.h"
#include "dang-glfw/Window.h"
// #include "dang-gl/Image/BorderedImageMipmapper.h"
#include "dang-gl/Image/Image.h"
#include "dang-gl/Image/PNGLoader.h"
#include "dang-gl/Math/MathConstants.h"
#include "dang-gl/Math/MathTypes.h"
#include "dang-gl/Objects/DataTypes.h"
#include "dang-gl/Objects/FBO.h"
#include "dang-gl/Objects/Program.h"
#include "dang-gl/Objects/RBO.h"
#include "dang-gl/Objects/Texture.h"
#include "dang-gl/Objects/VAO.h"
#include "dang-gl/Objects/VBO.h"
#include "dang-gl/Rendering/Camera.h"
#include "dang-gl/Rendering/Renderable.h"
#include "dang-gl/Texturing/MultiTextureAtlas.h"
#include "dang-gl/Texturing/TextureAtlas.h"
#include "dang-glfw/GLFW.h"
#include "dang-glfw/Window.h"
#include "dang-lua/State.h"
//#include "dang-lua/stl-vector.h"

#include "dang-math/bounds.h"
#include "dang-math/interpolation.h"
//#include "dang-math/lua-geometry.h"

// #include "dang-math/lua-enums.h"
// #include "dang-math/lua-geometry.h"
// #include "dang-math/lua-vector-matrix.h"
#include "dang-math/marchingcubes.h"
//#include "dang-math/noise.h"

//#include "dang-utils/lua-event.h"

#include "dang-example/global.h"

struct CubeData {
    dgl::vec3 pos;
    dgl::vec2 texcoord;
};

struct OffsetData {
    dgl::vec3 offset;
};

struct ColorData {
    dgl::vec3 color;
};

struct BillboardData {
    dgl::vec2 texcoord;
};

struct PointData {
    dgl::vec3 pos;
    GLfloat radius{};
    dgl::vec3 color;
};

struct ModelData {
    dgl::vec3 pos;
    dgl::vec3 color;
    dgl::vec3 normal;
    GLfloat mipmaps{};
};

class TestCube : public dgl::Renderable {
public:
    TestCube(dgl::Program& program, dgl::vec3 pos)
        : vao_(program, cube_vbo_, offset_vbo_, color_vbo_)
        , pos_(pos)
    {
        cube_vbo_.setLabel("TestCube-Cube-VBO");
        offset_vbo_.setLabel("TestCube-Offset-VBO");
        color_vbo_.setLabel("TestCube-Color-VBO");
        vao_.setLabel("TestCube-VAO");

        generate();
    }

    static std::unique_ptr<TestCube> create(dgl::Program& program, dgl::vec3 pos)
    {
        return std::make_unique<TestCube>(program, pos);
    }

    void generate()
    {
        std::vector<CubeData> cube_data;
        cube_data.reserve(36);
        for (auto& plane : dgl::cube_planes) {
            for (auto& texcoord : dgl::quad_tex_coords) {
                auto pos = plane[texcoord];
                CubeData data;
                data.pos = pos_ * 10.0f + (pos + 0.5f) * 1.0f;
                data.texcoord = texcoord;
                cube_data.push_back(data);
            }
        }
        cube_vbo_.generate(cube_data);

        std::vector<OffsetData> offset_data;
        for (auto& offset : dmath::ibounds3(-6, 6)) {
            offset_data.push_back({static_cast<dgl::vec3>(offset) * 2.0f});
        }
        offset_vbo_.generate(offset_data);

        std::vector<ColorData> color_data;
        /*
        for (int i = 0; i < 12 * 12 * 12 / 6; i++) {
            color_data.push_back({dgl::vec3(1, 0, 0)});
            color_data.push_back({dgl::vec3(1, 1, 0)});
            color_data.push_back({dgl::vec3(0, 1, 0)});
            color_data.push_back({dgl::vec3(0, 1, 1)});
            color_data.push_back({dgl::vec3(0, 0, 1)});
            color_data.push_back({dgl::vec3(1, 0, 1)});
        }
        */

        for (int i = 0; i < 12 * 12 * 12; i++) {
            color_data.push_back({dgl::vec3(1)});
        }
        color_vbo_.generate(color_data);
    }

    dgl::SharedTransform transform() const override { return transform_; }

    dgl::Program& program() const override { return vao_.program(); }

    void draw() const override { vao_.draw(); }

private:
    dgl::SharedTransform transform_ = dgl::Transform::create();
    dgl::VBO<CubeData> cube_vbo_;
    dgl::VBO<OffsetData> offset_vbo_;
    dgl::VBO<ColorData> color_vbo_;
    dgl::VAO<CubeData, OffsetData, ColorData> vao_;
    dgl::vec3 pos_;
};

class TestPoints : public dgl::Renderable {
public:
    TestPoints(dgl::Program& program)
        : vao_(program, billboard_vbo_, point_vbo_)
    {
        billboard_vbo_.setLabel("TestPoints-Billboard-VBO");
        point_vbo_.setLabel("TestPoints-Point-VBO");
        vao_.setLabel("TestPoints-VAO");

        generate();
    }

    static std::unique_ptr<TestPoints> create(dgl::Program& program) { return std::make_unique<TestPoints>(program); }

    void generate()
    {
        std::vector<BillboardData> billboard_data;
        billboard_data.reserve(6);
        for (auto& texcoord : dgl::centered_tex_coords) {
            billboard_data.push_back({texcoord});
        }
        billboard_vbo_.generate(billboard_data);

        std::vector<PointData> point_data;

        std::array<dmath::vec3, 6> colors{{{1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1}, {0, 0, 1}, {1, 0, 1}}};

        // for (auto& offset : dmath::ibounds3(-1, 1)) {
        // for (auto& offset : dmath::ibounds3(-13, 11)) {
        // for (auto& offset : dmath::ibounds3(0, 1)) {
        for (std::size_t facing = 0; facing < 6; facing += 1) {
            auto bounds = dmath::sbounds3{{5, 6, 1}}.facing(facing, dmath::BoundsClipInfo{true, true});
            for (const auto& offset : bounds) {
                PointData data;
                data.color = colors[facing];
                data.pos = dgl::vec3{offset + 1} * 1.0f + 0.5f;
                data.pos.x() += facing * 0.05f;
                // data.pos = dgl::vec3{offset} * 0.2f + 0.1f;
                data.radius = 0.2f;
                point_data.push_back(data);
            }
        }
        point_vbo_.generate(point_data);
    }

    dgl::SharedTransform transform() const override { return transform_; }

    dgl::Program& program() const override { return vao_.program(); }

    void draw() const override { vao_.draw(); }

private:
    dgl::SharedTransform transform_ = dgl::Transform::create();
    dgl::VBO<BillboardData> billboard_vbo_;
    dgl::VBO<PointData> point_vbo_;
    dgl::VAO<BillboardData, PointData> vao_;
};

class TestModel : public dgl::Renderable {
public:
    TestModel(dglfw::Window& window, dgl::Program& program)
        : vao_(program, vbo_, dgl::BeginMode::Triangles)
    {
        vbo_.setLabel("Model-VBO");
        vao_.setLabel("Model-VAO");

        generate();

        /*
        window.onScroll.append([&](dglfw::ScrollInfo info) {
            outer_factor_ += dgl::vec2{info.offset}.y() * 0.001f;
            generate();
        });
        */

        // window.onUpdate.append([&] {
        // time_ = static_cast<float>(dglfw::instance().time());
        // generate();
        //});

        window.onKey.append([&](dglfw::KeyInfo info) {
            if (info.action != dglfw::KeyAction::Press)
                return;
            if (info.key == dglfw::Key::Z) {
                smooth_normals_ = !smooth_normals_;
                generate();
            }
            else if (info.key == dglfw::Key::V) {
                better_normals_ = !better_normals_;
                generate();
            }
        });
    }

    static std::unique_ptr<TestModel> create(dglfw::Window& window, dgl::Program& program)
    {
        return std::make_unique<TestModel>(window, program);
    }

    bool getPoint(dgl::vec4 point)
    {
        // static std::map<dgl::vec4, bool> points;

        // auto iter = points.find(point);
        // if (iter != points.end())
        // return iter->second;

        // auto v = dmath::vec4(point);

        auto n1 = -1.0f; // dmath::noise::simplex(dmath::vec4(v.xyz() * 0.1f, 0.0f));
        // auto n2 = dmath::noise::simplex(dmath::vec4(v.xyz() * 0.1f, v.w()));
        // return points[point] =
        return n1 < 0.0f && point.xyz().length() < radius();

        /*
        auto r = [](auto vec) {
            vec = dgl::vec3{
                std::sin(vec.x() + 82359.32498f), std::cos(vec.y() * 728357.72839f), std::sin(vec.z() * 75832.2384f)};
            auto result = std::abs(vec.dot({68.238749f, 6.23894f, 15.74985f}));
            return result - std::floor(result);
        };

        return point.abs().lessThan(radius()).all() && point.length() > radius() * 1.1f &&
                   point.length() < radius() * outer_factor_ * (0.9f + 0.1f) ||
               point.length() < radius() * 0.9f;
        */
    }

    int radius() { return 8; }

    void generate()
    {
        std::vector<ModelData> data;

        std::map<dgl::vec3, dgl::vec3> normals;

        auto add = [&](dgl::vec3 pos, dgl::vec3 normal) {
            normals[pos] += normal;
            // auto color = dgl::vec3{r(pos), r(pos + 2.5489f), r(pos + 5.234f)};
            // auto color = dgl::vec3{0.5, 0.2, 1.0};
            auto color = dgl::vec3{1.0};
            data.push_back(ModelData{pos, color, normal, 9.0f});
        };

        const dmath::MarchingCubes<false> mc;

        auto start = std::chrono::high_resolution_clock::now();
        for (const auto& pos : dmath::ibounds3{-radius(), radius()}) {
            dmath::Corners3 corners;
            for (auto corner : dutils::enumerate<dmath::Corner3>)
                corners.set(corner,
                            getPoint(dmath::vec4(dmath::vec3(pos + dmath::corner_vector_3[corner]), time_ * 0.1f)));
            for (auto plane_info : mc[corners]) {
                auto plane = plane_info.makePlane(0.5f);
                auto normal = plane.normal();

                if (smooth_normals_ && better_normals_) {
                    add(dgl::vec3{pos} + plane[{0, 0}], plane.innerRadians(0) * normal);
                    add(dgl::vec3{pos} + plane[{1, 0}], plane.innerRadians(1) * normal);
                    add(dgl::vec3{pos} + plane[{0, 1}], plane.innerRadians(2) * normal);
                }
                else {
                    add(dgl::vec3{pos} + plane[{0, 0}], normal);
                    add(dgl::vec3{pos} + plane[{1, 0}], normal);
                    add(dgl::vec3{pos} + plane[{0, 1}], normal);
                }
            }
        }
        auto stop = std::chrono::high_resolution_clock::now();

        if (smooth_normals_) {
            for (auto& [key, normal] : normals)
                normal = normal.normalize();

            for (auto& datum : data)
                datum.normal = normals[datum.pos];
        }

        vbo_.generate(data);

        std::cout << outer_factor_ << " -> "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << "ms\n";
        std::cout << "Smooth Normals: " << std::boolalpha << smooth_normals_ << '\n';
        std::cout << "Better Normals: " << std::boolalpha << better_normals_ << '\n';
        std::cout << '\n';
    }

    dgl::Program& program() const override { return vao_.program(); }

    void draw() const override { vao_.draw(); }

private:
    bool smooth_normals_ = true;
    bool better_normals_ = true;
    float outer_factor_ = 1.2f;
    float time_ = 0.0f;
    dgl::VBO<ModelData> vbo_;
    dgl::VAO<ModelData> vao_;
};

class Noisy {
public:
    Noisy()
        : Noisy("unnamed")
    {}

    Noisy(std::string name) noexcept
        : name_(std::move(name))
    {
        std::cout << name_ << " constructor\n";
    }

    ~Noisy() noexcept { std::cout << name_ << " destructor\n"; }

    Noisy(const Noisy& other) noexcept
        : name_(other.name_)
    {
        std::cout << name_ << " copy-constructor\n";
    }

    Noisy(Noisy&& other) noexcept
        : name_(other.name_)
    {
        other.moved_ = true;
        std::cout << name_ << " move-constructor\n";
    }

    Noisy& operator=(const Noisy& other) noexcept
    {
        name_ = other.name_;
        std::cout << name_ << " copy-assignment\n";
        return *this;
    }

    Noisy& operator=(Noisy&& other) noexcept
    {
        name_ = other.name_;
        other.moved_ = true;
        std::cout << name_ << " move-assignment\n";
        return *this;
    }

    void rename(std::string name)
    {
        std::cout << name_ << " -> " << name << "\n";
        name_ = name;
    }

private:
    std::string name_;
    bool moved_ = false;
};

int main()
try {
    dglfw::GLFW glfw;

    dglfw::WindowInfo window_info;
    window_info.title = "Hello World!";

    // window_info.size = {50, 50};
    // window_info.framebuffer.samples = 32;
    // window_info.transparent_framebuffer = true;
    // window_info.decorated = false;

    dglfw::Window window(window_info);
    window.setCursorMode(dglfw::CursorMode::Disabled);
    window.maximize();

    auto& context = window.context();

    context->debug_output = true;
    context->debug_output_synchronous = true;

    context.onGLDebugMessage.append([](const dgl::DebugMessageInfo& info) {
        if (info.severity == dgl::DebugSeverity::Notification)
            return;
        std::cout << info.message << std::endl;
    });

    context->depth_test = true;
    context->cull_face = true;
    context->blend = true;
    context->blend_func = {dgl::BlendFactorSrc::SrcAlpha, dgl::BlendFactorDst::OneMinusSrcAlpha};

    window.setVSync(dglfw::VSync::Enabled);
    window.setFinishAfterSwap(true);

    /*
    auto color_rbo = dgl::RBO::color(window.framebufferSize(), 0);
    auto depth_stencil_rbo = dgl::RBO::depthStencil(window.framebufferSize(), 0);
    */

    /*
    dgl::FBO fbo;
    fbo.setLabel("MainFBO");
    fbo.attach(color_rbo, fbo.colorAttachment(0));
    fbo.attach(depth_stencil_rbo, fbo.depthStencilAttachment());
    fbo.checkComplete();
    */

    dgl::Program cube_program;
    cube_program.setLabel("Cube-Program");
    cube_program.addIncludeFromFile("shaders/quaternion.glsl");
    cube_program.addShaderFromFile(dgl::ShaderType::Vertex, "shaders/cube.vert");
    cube_program.addShaderFromFile(dgl::ShaderType::Fragment, "shaders/cube.frag");
    cube_program.link({"v_pos", "v_texcoord"}, {{1, {"v_offset"}}, {1, {"v_color"}}});

    dgl::Program model_program;
    model_program.setLabel("Model-Program");
    model_program.addIncludeFromFile("shaders/quaternion.glsl");
    model_program.addShaderFromFile(dgl::ShaderType::Vertex, "shaders/model.vert");
    model_program.addShaderFromFile(dgl::ShaderType::Fragment, "shaders/model.frag");
    model_program.link({"v_pos", "v_color", "v_normal", "v_maxlod"});

    dgl::Program billboard_program;
    billboard_program.setLabel("Billboard-Program");
    billboard_program.addIncludeFromFile("shaders/quaternion.glsl");
    billboard_program.addShaderFromFile(dgl::ShaderType::Vertex, "shaders/billboard.vert");
    billboard_program.addShaderFromFile(dgl::ShaderType::Fragment, "shaders/billboard.frag");
    billboard_program.link({"v_texcoord"}, {{1, {"v_pos", "v_radius", "v_color"}}});

    using Image = dgl::Image<2, dgl::PixelFormat::RGB>;
    // using BorderedImage = dgl::BorderedImage<2, Image::pixel_format>;

    using Atlas = dgl::TextureAtlas<Image::pixel_format, Image::pixel_type, Image::row_alignment>;

    std::vector<Atlas::TileHandle> tiles;
    std::size_t tile_index = 0;

    auto atlas = [&] {
        Atlas atlas;

        /*
        for (const auto& entry : fs::directory_iterator("PngSuite")) {
            if (entry.path().extension() != ".png")
                continue;
            if (entry.path().filename().string().front() == 'x')
                continue;

            tiles.push_back(atlas.add(Image::loadFromPNG(entry), dgl::TextureAtlasTileBorderGeneration::None));
            tiles.push_back(atlas.add(Image::loadFromPNG(entry), dgl::TextureAtlasTileBorderGeneration::Positive));
            tiles.push_back(atlas.add(Image::loadFromPNG(entry), dgl::TextureAtlasTileBorderGeneration::All));
        }
        */

        // for (int i = 0; i < 100; i++) {
        // tiles.push_back(atlas.add(Image::loadFromPNG("textures/dirt.png")));
        // tiles.push_back(atlas.add(Image::loadFromPNG("textures/grass.png")));
        // tiles.push_back(atlas.add(Image::loadFromPNG("textures/pebbles.png")));
        // auto stone = Image::loadFromPNG("textures/realstone.png");

        // auto half_stone = dgl::image::filter(stone, stone.size() / 2, dmath::mat2{{{1, 1}, {1, 1}}} / 4);

        // tiles.push_back(atlas.add(std::move(stone), dgl::TextureAtlasTileBorderGeneration::None));
        // tiles.push_back(atlas.add(std::move(half_stone), dgl::TextureAtlasTileBorderGeneration::None));
        // tiles.push_back(atlas.add(half_stone, dgl::TextureAtlasTileBorderGeneration::Positive));
        // tiles.push_back(atlas.add(half_stone, dgl::TextureAtlasTileBorderGeneration::All));
        // tiles.push_back(atlas.add(Image::loadFromPNG("textures/rectangles.png")));
        // tiles.push_back(atlas.add(Image::loadFromPNG("textures/test.png")));
        //}

        /*
        for (int i = 0; i < 1; i++) {
            auto noise_image = Image(Image::Size{512});
            for (auto pos : dmath::sbounds2(noise_image.size())) {
                auto fpos = (dmath::vec2(pos) - 0.0f) / 16.0f;
                dmath::vec2 value(fpos.x(), fpos.y());
                auto x = 1.0 - std::abs(dmath::noise::simplex(value, dmath::noise::PermuteFactors::fromSeed(i)));
                noise_image[pos] = Image::Pixel(static_cast<std::uint8_t>(255.0 * x * x));
            }
            tiles.push_back(atlas.add({BorderedImage::addBorder(dgl::ImageBorderWrapPositive{}, noise_image),
                                       dgl::bordered_image_mipmapper<>}));
        }
        //*/

        /*
        auto image = Image::loadFromPNG("textures/realstone.png", 0, 0);
        tiles.push_back(atlas.add({BorderedImage::addBorder(dgl::ImageBorderSolid<dgl::PixelFormat::RGB>{255}, image),
                                   dgl::bordered_image_mipmapper<>}));
        tiles.push_back(
            atlas.add({BorderedImage::addBorder(dgl::ImageBorderWrapBoth{}, image), dgl::bordered_image_mipmapper<>}));
        tiles.push_back(atlas.add(
            {BorderedImage::addBorder(dgl::ImageBorderWrapPositive{}, image), dgl::bordered_image_mipmapper<>}));
        //*/
        auto image = Image::loadFromPNG("textures/realstone.png", 0, 0);
        tiles.push_back(atlas.add(image));

        return atlas;
    }();
    //.freeze();

    // TODO: Problem if mipmapped and non-mipmapped image are on the same layer

    atlas.updateTexture();

    atlas.texture().setMinFilter(dgl::TextureMinFilter::LinearMipmapLinear);
    // atlas.texture().setMagFilter(dgl::TextureMagFilter::Linear);

    model_program.uniformSampler("diffuse_map") = atlas.texture();

    // atlas.add("dirt", dgl::Image2D::loadFromPNG("textures/dirt.png"));
    // atlas.add("grass", dgl::Image2D::loadFromPNG("textures/grass.png"));
    // atlas.add("pebbles", dgl::Image2D::loadFromPNG("textures/pebbles.png"));
    // atlas.add("real stone", dgl::Image2D::loadFromPNG("textures/realstone.png"));
    // atlas.add("rectangles", dgl::Image2D::loadFromPNG("textures/rectangles.png"));
    // atlas.add("test", dgl::Image2D::loadFromPNG("textures/test.png"));
    // atlas.generateTexture();

    /*
    dgl::Texture2DArray model_textures({512, 512, 6});
    model_textures.modify(dgl::Image2D::loadFromPNG("textures/dirt.png"), {0, 0, 0});
    model_textures.modify(dgl::Image2D::loadFromPNG("textures/grass.png"), {0, 0, 1});
    model_textures.modify(dgl::Image2D::loadFromPNG("textures/pebbles.png"), {0, 0, 2});
    model_textures.modify(dgl::Image2D::loadFromPNG("textures/realstone.png"), {0, 0, 3});
    model_textures.modify(dgl::Image2D::loadFromPNG("textures/rectangles.png"), {0, 0, 4});
    model_textures.modify(dgl::Image2D::loadFromPNG("textures/test.png"), {0, 0, 5});
    model_textures.setLabel("Model-Textures");
    model_textures.generateMipmap();
    model_textures.setWrapS(dgl::TextureWrap::Repeat);
    model_textures.setWrapT(dgl::TextureWrap::Repeat);
    */

    cube_program.uniformSampler("diffuse_map") = atlas.texture();
    // auto& cube_texture_index = cube_program.uniform<GLint>("texture_index");

    auto& time = model_program.uniform<GLfloat>("time");
    auto& texture_index = model_program.uniform<int>("texture_index");
    auto& uv_bounds = model_program.uniform<dgl::mat2>("uv_bounds");

    const auto& tile = tiles[tile_index];
    texture_index = tile.layer();
    // uv_bounds = dgl::mat2::fromBounds(dgl::bounds2{1});
    uv_bounds = dgl::mat2::fromBounds(tile.bounds());

    std::vector<dgl::UniqueRenderable> renderables;

    // for (const auto& v : dmath::ibounds3(0, 1)) {
    // dgl::vec3 pos = dgl::vec3(v);
    // renderables.push_back(TestCube::create(cube_program, pos * 0.02f));
    //}
    // renderables.push_back(TestPoints::create(billboard_program));
    renderables.push_back(TestModel::create(window, model_program));

    auto camera = dgl::Camera::perspective(context, 90, {0.1f, 1000.0f});

    dgl::dquat pos = dgl::dquat::fromTranslation({0, 8, 0});

    float t = 0;
    float fps_update = 0;
    window.onUpdate.append([&] {
        t += window.deltaTime() / 10.0f;
        fps_update -= window.deltaTime();
        if (fps_update <= 0) {
            window.setTitle(std::to_string(static_cast<int>(std::round(window.fps()))) + " FPS");
            fps_update = 0.5;
            // texture_index = rand() % 6;
        }
        time = static_cast<float>(glfw.time());

        auto cursor_pos = window.normalizedCursorPos() * 90;
        const auto& look = dgl::dquat::fromEulerYX({-cursor_pos.x(), cursor_pos.y()});

        auto speed = window.deltaTime() * 15;

        if (window.isKeyDown(dglfw::Key::A))
            pos *= look.inverseFast() * dgl::dquat::fromTranslation({-speed, 0, 0}) * look;
        if (window.isKeyDown(dglfw::Key::D))
            pos *= look.inverseFast() * dgl::dquat::fromTranslation({speed, 0, 0}) * look;
        if (window.isKeyDown(dglfw::Key::LeftShift))
            pos *= look.inverseFast() * dgl::dquat::fromTranslation({0, -speed, 0}) * look;
        if (window.isKeyDown(dglfw::Key::Space))
            pos *= look.inverseFast() * dgl::dquat::fromTranslation({0, speed, 0}) * look;
        if (window.isKeyDown(dglfw::Key::W))
            pos *= look.inverseFast() * dgl::dquat::fromTranslation({0, 0, -speed}) * look;
        if (window.isKeyDown(dglfw::Key::S))
            pos *= look.inverseFast() * dgl::dquat::fromTranslation({0, 0, speed}) * look;

        dgl::dquat transform;
        // transform *= dgl::dquat::fromTranslation({0, 0, 240});
        // transform *= dgl::dquat::fromTranslation({0, 0, -1});
        transform *= look;
        transform *= pos;
        camera.transform()->setOwnTransform(transform);
    });

    window.onKey.append([&](dglfw::KeyInfo info) {
        if (info.action != dglfw::KeyAction::Press)
            return;

        if (info.key == dglfw::Key::F)
            context->polygon_mode_front =
                context->polygon_mode_front == dgl::PolygonMode::Fill ? dgl::PolygonMode::Line : dgl::PolygonMode::Fill;
        if (info.key == dglfw::Key::L)
            context->depth_clamp = !context->depth_clamp;
    });

    window.onScroll.append([&](dglfw::ScrollInfo info) {
        tile_index += static_cast<std::size_t>(info.offset.y());
        tile_index = std::clamp<std::size_t>(tile_index, 0, tiles.size() - 1);

        const auto& tile = tiles[tile_index];
        texture_index = tile.layer();
        uv_bounds = dgl::mat2::fromBounds(tile.bounds());
    });

    /*
    window.onFramebufferResize.append([&] {
        fbo.detach(fbo.colorAttachment(0));
        fbo.detach(fbo.depthStencilAttachment());

        color_rbo.destroy();
        depth_stencil_rbo.destroy();

        color_rbo = dgl::RBO::color(window.framebufferSize());
        depth_stencil_rbo = dgl::RBO::depthStencil(window.framebufferSize());

        fbo.attach(color_rbo, fbo.colorAttachment(0));
        fbo.attach(depth_stencil_rbo, fbo.depthStencilAttachment());

        fbo.checkComplete();
    });
    */

    // fbo.bind();
    window.onRender.append([&] {
        // fbo.clear();
        camera.render(renderables);
        // fbo.blitToDefault();
    });

    window.run();
}
catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
}
catch (...) {
    std::cerr << "Unknown exception" << std::endl;
}
