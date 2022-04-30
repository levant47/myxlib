typedef u32 Pixel;

const Pixel BLACK = 0;
const Pixel WHITE = -1;

struct Image
{
    Pixel* data;
    u64 width;
    u64 height;

    static Image allocate(u64 width, u64 height)
    {
        Image result;
        result.width = width;
        result.height = height;
        result.data = (Pixel*)default_allocate(width * height * sizeof(Pixel));
        return result;
    }

    void deallocate()
    {
        default_deallocate(data);
    }
};
