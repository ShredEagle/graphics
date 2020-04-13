#pragma once

#include "../../Image.h"

#include <math/Vector.h>


static constexpr std::size_t gChunkSize = 256/*kB*/ * 1024/*B*/;


namespace ad {
namespace detail {


    using Rgb = math::sdr::Rgb;


    struct Ppm
    {
        static math::Size<2, int> ReadHeader(std::istream & aIn)
        {
            char magic1{0}, magic2{0};
            aIn >> magic1 >> magic2;
            if (magic1 != 'P' || magic2 != '6')
            {
                throw std::runtime_error("Invalid header for PPM content");
            }

            int width{-1}, height{-1}, maxValue{0};
            aIn >> width >> height >> maxValue;

            if (width <= 0 || height <= 0)
            {
                throw std::runtime_error("Invalid PPM dimensions");
            }
            if (maxValue != 255)
            {
                throw std::runtime_error("Unhandled PPM maximum value of " + std::to_string(maxValue));
            }

            aIn.get(); // consume the one byte appearing after max value and before raster data
                       // (carriage return or line feed)

            return {width, height};
        }

        static Image<Rgb> Read(std::istream & aIn)
        {
            auto dimensions = ReadHeader(aIn);

            std::size_t remainingBytes = dimensions.area() * sizeof(Rgb);
            auto data = std::make_unique<char[]>(remainingBytes);
            char * currentDestination = data.get();

            while (remainingBytes)
            {
                int readSize = std::min(remainingBytes, gChunkSize);
                if (!aIn.read(currentDestination, readSize).good())
                {
                    // If the stream is not good, but its converts to "true", it means it reached eof
                    // see: https://en.cppreference.com/w/cpp/io/basic_ios/good#see_also
                    throw std::runtime_error(std::string{"Invalid PPM content: "} +
                        (aIn ? "truncated content" : "read error"));
                }
                remainingBytes -= readSize;
                currentDestination += readSize;
            }

            if (aIn.peek(), !aIn.eof())
            {
                throw std::runtime_error("Invalid PPM content: trailing data");
            }

            return Image(dimensions, std::move(data));
        }

        static void Write(std::ostream & aOut, const Image<Rgb> & aImage)
        {
            if(!aOut.good())
            {
                throw std::runtime_error("Output stream is not valid for writing");
            }

            aOut << "P6\n"
                << aImage.width() << ' ' << aImage.height() << '\n'
                << "255\n"
                ;

            std::size_t remainingBytes = aImage.size_bytes();
            const char * currentSource = reinterpret_cast<const char *>(aImage.data());

            while (remainingBytes)
            {
                int writeSize = std::min(remainingBytes, gChunkSize);
                if (!aOut.write(currentSource, writeSize).good())
                {
                    throw std::runtime_error("Error writing PPM pixel data to stream");
                }
                remainingBytes -= writeSize;
                currentSource += writeSize;
            }
        }
    };

} // namespace detail
} // namespace ad
