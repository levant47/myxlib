struct InputState
{
    static const u64 padding = 5; // pixels
    static const u64 cursor_width = 2; // pixels

    bool is_in_focus;
    Vector2<u64> position;
    Vector2<u64> dimensions;
    String text;
    Pixel text_color;
    u64 font_size;
    u64 timer;

    static InputState construct(Vector2<u64> position, Vector2<u64> dimensions, u64 font_size, Pixel text_color = BLACK)
    {
        InputState result;
        result.is_in_focus = true;
        result.position = position;
        result.dimensions = dimensions;
        result.text = String::allocate();
        result.text_color = text_color;
        result.font_size = font_size;
        result.timer = 0;
        return result;
    }
};

// void render_line(Image image, Vector2<u64> start, Vector2<u64> end, u64 width, Pixel color)
// {
//     for (u64 y = start.y; y != end.y; y++)
//     {
//         for (u64 x = start.x; y != end.x; x++)
//         {
//             for (u64 i = x - width / 2;
//             auto pixel_i = y * image.width + x;
//             image.data[pixel_i] = 
//         }
//     }
// }

void render_box(Image image, Vector2<u64> position, Vector2<u64> dimensions, u64 width, Pixel color)
{
    for (u64 y = position.y; y < position.y + dimensions.y; y++)
    {
        for (u64 x = position.x; x < position.x + dimensions.x; x++)
        {
            if (x == position.x || y == position.y || x == position.x + dimensions.x - 1 || y == position.y + dimensions.y - 1)
            {
                auto pixel_i = y * image.width + x;
                image.data[pixel_i] = color;
            }
        }
    }
}

void render_input_text(InputState state, Image target_image)
{
    auto text_width = state.text.size * GLYPH_WIDTH * state.font_size / GLYPH_WIDTH / 2 + state.is_in_focus * InputState::cursor_width;
    auto input_width = state.dimensions.x - InputState::padding * 2;
    if (text_width <= input_width)
    {
        render_text(
            state.text,
            state.text_color,
            target_image,
            state.position + InputState::padding,
            state.font_size
        );
    }
    else
    {
        auto text_height = GLYPH_HEIGHT * state.font_size / GLYPH_HEIGHT;
        auto buffer_image = Image::allocate(text_width, text_height);
        buffer_image.clear(WHITE);

        render_text(state.text, state.text_color, buffer_image, Vector2<u64>::construct(0, 0), state.font_size);

        for (
            u64 x = text_width - 1, target_x = state.position.x + state.dimensions.x - InputState::padding - state.is_in_focus * InputState::cursor_width;
            target_x >= state.position.x + InputState::padding;
            x--, target_x--)
        {
            for (u64 y = 0, target_y = state.position.y + InputState::padding; y < text_height; y++, target_y++)
            {
                auto buffer_pixel_i = y * text_width + x;
                auto target_pixel_i = target_y * target_image.width + target_x;
                target_image.data[target_pixel_i] = buffer_image.data[buffer_pixel_i];
            }
        }

        buffer_image.deallocate();
    }
}

void render_input_cursor(InputState state, Image image)
{
    if (state.is_in_focus && state.timer % 60 < 30)
    {
        auto text_width = state.text.size * GLYPH_WIDTH * state.font_size / GLYPH_WIDTH / 2;
        auto cursor_position_x = state.position.x + InputState::padding + min(text_width, state.dimensions.x - InputState::padding * 2 - InputState::cursor_width);
        for (u64 y = state.position.y + InputState::padding; y < state.position.y + InputState::padding + state.font_size; y++)
        {
            for (u64 x = cursor_position_x; x < cursor_position_x + InputState::cursor_width; x++)
            {
                image.data[y * image.width + x] = BLACK;
            }
        }
    }
}

void render_input(InputState* state, List<X11Event> events, Image image)
{
    for (u64 i = 0; i < events.size; i++)
    {
        if (events.data[i].type == X11EventTypeKeyPress)
        {
            auto event = *(X11EventKeyPress*)(&events.data[i]);
            if (event.key_code == X11KeyCodeBackspace)
            {
                if (state->text.size != 0)
                {
                    state->text.pop();
                }
            }
            else
            {
                auto maybe_char = event.to_char();
                if (maybe_char.has_data)
                {
                    state->text.push(maybe_char.value);
                }
            }
            state->timer = 0; // reset timer on key press so that the cursor isn't blinking while typing
        }
    }

    render_box(image, state->position, state->dimensions, 0, BLACK);
    Vector2<u64> text_position;
    text_position.y = state->position.y + (state->dimensions.y - state->font_size) / 2;
    text_position.x = state->position.y + InputState::padding;
    // render_text(state->text, state->text_color, image, text_position, state->font_size);
    render_input_text(*state, image);
    render_input_cursor(*state, image);

    state->timer++;
}
