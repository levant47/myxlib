#include <mystd/include_linux.h>

#pragma pack(push, 1)

#include "x11.cpp"
#include "renderer.cpp"
#include "text_renderer.cpp"

const char X11_SOCKET_PATH[] = "/tmp/.X11-unix/X0";
CStringView XAUTHORITY_PATH = "/run/user/1000/gdm/Xauthority"; // TODO: make this user-independent
const s16 TARGET_X11_MAJOR_VERSION = 11;
const s16 TARGET_X11_MINOR_VERSION = 0;
const u16 DEPTH = 24;
const u32 WINDOW_WIDTH = 512 * 2;
const u32 WINDOW_HEIGHT = 512;
const u64 X11_MAX_REQUEST_SIZE = 512 * 100 * 4; // in bytes

void put_image_in_chunks(
    Descriptor x11_socket,
    u32 window_id,
    u32 graphics_context_id,
    Pixel* image,
    u64 width,
    u64 height
)
{
    u64 put_image_request_body_size = width * height * sizeof(Pixel); // bytes
    X11PutImageRequestHeader put_image_request_header;
    put_image_request_header.type = X11RequestTypePutImage;
    put_image_request_header.format = X11ImageFormatZPixmap;
    put_image_request_header.drawable_id = window_id;
    put_image_request_header.graphics_context_id = graphics_context_id;
    put_image_request_header.depth = DEPTH;

    u64 usual_batch_size = 0;
    u64 line_size = width * sizeof(Pixel); 
    u64 total_image_size = width * height * sizeof(Pixel);
    while (usual_batch_size + line_size < min(X11_MAX_REQUEST_SIZE, total_image_size))
    {
        usual_batch_size += line_size;
    }

    u64 bytes_sent = 0;
    u64 total_bytes = width * height * sizeof(Pixel);
    u64 height_counter = 0;
    while (bytes_sent != total_bytes)
    {
        auto batch_size = min(usual_batch_size, total_bytes - bytes_sent);
        put_image_request_header.request_size_in_dwords = (sizeof(put_image_request_header) + batch_size) / 4;
        put_image_request_header.width = width;
        auto batch_height = batch_size / width / sizeof(Pixel);
        put_image_request_header.height = batch_height;
        put_image_request_header.position_x = 0;
        put_image_request_header.position_y = height_counter;
        put_image_request_header.left_pad = 0;

        auto write_put_image_request_header_result = write(x11_socket, &put_image_request_header, sizeof(put_image_request_header));
        assert(write_put_image_request_header_result == sizeof(put_image_request_header), "Failed to write put image request header");

        auto write_put_image_request_body_result = write(x11_socket, (byte*)image + bytes_sent, batch_size);
        assert(write_put_image_request_body_result == batch_size, "Failed to write put image request body");

        height_counter += batch_height;
        bytes_sent += batch_size;
    }
}

extern "C" void _start()
{
    auto x11_socket = socket(SocketDomainUnix, SocketTypeTcp);
    assert(x11_socket != -1, "Failed to open X11 socket");

    UnixSocketAddress x11_socket_address;
    x11_socket_address.family = SocketDomainUnix;
    copy_memory(X11_SOCKET_PATH, sizeof(X11_SOCKET_PATH) + 1 /* include zero terminator */, &x11_socket_address.path);
    auto connect_result = connect(x11_socket, &x11_socket_address, sizeof(x11_socket_address));
    assert(connect_result == 0, "Connect failed");

    auto xauthority_contents = read_whole_file(XAUTHORITY_PATH).unwrap("Failed to read Xauthority");
    auto x11_cookie = xauthority_contents.data + xauthority_contents.size - MIT_COOKIE_SIZE;

    // connection request
    X11ConnectionRequest connection_request;
    connection_request.order = X11EndiannessLittle;
    connection_request.protocol_major_version = TARGET_X11_MAJOR_VERSION;
    connection_request.protocol_minor_version = TARGET_X11_MINOR_VERSION;
    connection_request.authorization_protocol_name_size = get_c_string_length(MIT_COOKIE_PROTOCOL_NAME);
    connection_request.authorization_protocol_data_size = MIT_COOKIE_SIZE;
    copy_memory(MIT_COOKIE_PROTOCOL_NAME, get_c_string_length(MIT_COOKIE_PROTOCOL_NAME), &connection_request.authorization_protocol_name);
    copy_memory(x11_cookie, MIT_COOKIE_SIZE, &connection_request.authorization_protocol_data);
    auto write_connection_request_result = write(x11_socket, &connection_request, sizeof(X11ConnectionRequest));
    assert(write_connection_request_result == sizeof(X11ConnectionRequest), "Failed to send connection request");

    // connection response header
    X11ConnectionResponseHeader connection_response_header;
    auto read_connection_response_header_result = read(x11_socket, &connection_response_header, sizeof(connection_response_header));
    assert(read_connection_response_header_result == sizeof(connection_response_header), "Failed to read connection response header");
    assert(connection_response_header.status == X11ConnectionStatusSuccess, "Failed to authenticate with X11");

    // connection response body
    u64 connection_response_body_size = connection_response_header.body_size_in_dwords * 4;
    auto connection_response_body = default_allocate(connection_response_body_size);
    auto read_connection_response_body_result = read(x11_socket, connection_response_body, connection_response_body_size);
    assert(read_connection_response_body_result == connection_response_body_size, "Failed to read connection response body");

    auto connection_response_body_initial = (X11ConnectionResponseBodyInitial*)connection_response_body;
    auto window_id = connection_response_body_initial->base_id;
    auto screen_id = *(u32*)(
        connection_response_body
            + sizeof(X11ConnectionResponseBodyInitial)
            + connection_response_body_initial->vendor_len
            + connection_response_body_initial->num_pixmap_formats * 8
    );

    default_deallocate(connection_response_body);

    // create window
    u32 create_window_request_body[3] =
    {
        0x00FFFF00, // background
        0x00FF0000, // border
        X11EventMarkExposure | X11EventMarkButtonPress, // events
    };
    X11CreateWindowRequestHeader create_window_request_header;
    create_window_request_header.type = X11RequestTypeCreateWindow;
    create_window_request_header.depth = DEPTH;
    create_window_request_header.request_size_in_dwords = (sizeof(create_window_request_header) + sizeof(create_window_request_body)) / 4;
    create_window_request_header.window_id = window_id;
    create_window_request_header.parent_id = screen_id;
    create_window_request_header.position_x = 0;
    create_window_request_header.position_y = 0;
    create_window_request_header.width = WINDOW_WIDTH;
    create_window_request_header.height = WINDOW_HEIGHT;
    create_window_request_header.border_width = 20;
    create_window_request_header.window_class = X11WindowClassCopyFromParent;
    create_window_request_header.visual_id = X11_VISUAL_ID_COPY_FROM_PARENT;
    create_window_request_header.value_mask = X11WindowAttributeBackgroundPixel | X11WindowAttributeBorderPixel | X11WindowAttributeEventMask;
    auto write_create_window_request_header_result = write(x11_socket, &create_window_request_header, sizeof(create_window_request_header));
    assert(write_create_window_request_header_result == sizeof(create_window_request_header), "Failed to write create window request header");

    auto write_create_window_request_body_result = write(x11_socket, &create_window_request_body, sizeof(create_window_request_body));
    assert(write_create_window_request_body_result == sizeof(create_window_request_body), "Failed to write create window request body");

    // map window
    X11MapWindowRequest map_window_request;
    map_window_request.type = X11RequestTypeMapWindow;
    map_window_request.request_size_in_dwords = sizeof(X11MapWindowRequest) / 4;
    map_window_request.window_id = window_id;
    auto write_map_window_request_result = write(x11_socket, &map_window_request, sizeof(map_window_request));
    assert(write_map_window_request_result == sizeof(map_window_request), "Failed to write map window request");

    // create graphics context
    u32 graphics_context_id = connection_response_body_initial->base_id + 1;
    X11CreateGraphicsContextRequest create_graphics_context_request;
    create_graphics_context_request.type = X11RequestTypeCreateGraphicsContext;
    create_graphics_context_request.request_size_in_dwords = sizeof(X11CreateGraphicsContextRequest) / 4;
    create_graphics_context_request.graphics_context_id = graphics_context_id;
    create_graphics_context_request.drawable_id = screen_id;
    create_graphics_context_request.value_mask = 0;
    auto write_create_graphics_context_request_result = write(x11_socket, &create_graphics_context_request, sizeof(create_graphics_context_request));
    assert(write_create_graphics_context_request_result == sizeof(create_graphics_context_request), "Failed to write create graphics context request");

    // have to do this before drawing anything because otherwise there is a risk that X server will skip the first frame
    X11EventExpose expose_event;
    auto read_expose_event_result = read(x11_socket, &expose_event, sizeof(expose_event));
    assert(read_expose_event_result == sizeof(expose_event), "Failed to read expose event");
    assert(expose_event.type == X11EventTypeExpose, "Expected expose event");

    initialize_fonts();

    // put image
    Pixel color_wheel[3] = { 0x00FF0000, 0x0000FF00, 0x000000FF };
    Pixel font_color = BLACK;
    u64 current_color_index = 0;
    u32 counter = 0;
    auto image = Image::allocate(WINDOW_WIDTH, WINDOW_HEIGHT);;
    while (true)
    {
        PollParameter poll_parameter;
        poll_parameter.descriptor = x11_socket;
        poll_parameter.requested_events = PollEventDataAvailable;
        auto poll_result = poll(&poll_parameter, 1, 0);
        assert(poll_result >= 0, "Failed to poll X11 socket for events");
        if (poll_result != 0)
        { // there are events available
            if (poll_parameter.returned_events & PollEventHangUp)
            { // prevent a crash due to a pipe fail (status code 141)
                break;
            }


            if (poll_parameter.returned_events & PollEventDataAvailable)
            {
                for (u64 i = 0; i < (u64)poll_result; i++)
                {
                    byte event_buffer[32];
                    auto read_event_result = read(x11_socket, event_buffer, sizeof(event_buffer));
                    assert(read_event_result == sizeof(event_buffer), "Failed to read event");

                    if (event_buffer[0] == X11EventTypeButtonPress)
                    {
                        current_color_index = (current_color_index + 1) % 6;
                        font_color = current_color_index % 2 == 0 ? BLACK : WHITE;
                    }

                    // print(i, ": ");
                    // for (u64 k = 0; k < sizeof(event_buffer); k++)
                    // {
                    //     print((u64)event_buffer[k], " ");
                    // }
                    // print("\n");
                }
            }
        }

        for (u64 y = 0; y < image.height; y++)
        {
            for (u64 x = 0; x < image.width; x++)
            {
                auto pixel_index = y * image.width + x;
                image.data[pixel_index] = color_wheel[current_color_index % 3];
            }
        }

        // render_text("Hello world!", BLACK, image, Vector2<u64>::construct(100, 100), 16 * 8);
        render_text(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", font_color, image, Vector2<u64>::construct(50, 50), 16 * 4);

        put_image_in_chunks(x11_socket, window_id, graphics_context_id, image.data, image.width, image.height);

        SleepTime sleep_time;
        sleep_time.seconds = 0;
        sleep_time.nanoseconds = 16 * 1000 * 1000; // ~60 FPS
        nanosleep(&sleep_time);

        counter++;
    }

    image.deallocate();

    close(x11_socket);

    print("Done\n");

    exit(0);
}
