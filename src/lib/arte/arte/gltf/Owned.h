#pragma once


#include <platform/Filesystem.h>

#include <iostream>
#include <optional>
#include <vector>


namespace ad {
namespace arte {


namespace gltf {


    struct Uri
    {
        enum class Type
        {
            Data,
            File,
        };

        Uri(std::string aString) :
            string{std::move(aString)}
        {
            type = (string.rfind("data:", 0) == 0) ? Type::Data : Type::File;
        }

        std::string string;
        Type type;
    };


    template <class T_indexed>
    struct Index
    {
        using Value_t = std::vector<T_indexed>::size_type;

        Index(Value_t aValue) : 
            value{aValue}
        {}

        Index(const Index & aIndex) : 
            Index{aIndex.value}
        {}

        Index & operator=(Value_t aValue)
        {
            value = aValue;
            return this;
        }

        operator Value_t() const
        { return value; }

        Value_t value;
    };

    template <class T_indexed>
    std::ostream & operator<<(std::ostream & aOut, const std::vector<T_indexed> & aIndexVector);


} // namespace gltf


template <class T_element>
class Const_Owned;


template <class T_element>
class Owned
{
    friend class Gltf;
    friend class Const_Owned<T_element>;

public:
    Owned(Gltf & aGltf, T_element & aElement, gltf::Index<T_element> aIndex);

    operator T_element &()
    { return mElement; }

    T_element * operator->()
    {
        return &mElement;
    }

    T_element & operator*()
    {
        return mElement;
    }

    gltf::Index<T_element> id() const
    {
        return mIndex;
    }

    template <class T_indexed>
    Owned<T_indexed> get(gltf::Index<T_indexed> aIndex);

    template <class T_indexed>
    Owned<T_indexed> get(gltf::Index<T_indexed> T_element::* aDataMember)
    {
        return get(mElement.*aDataMember);
    }

    template <class T_indexed>
    Owned<T_indexed> get(std::optional<gltf::Index<T_indexed>> T_element::* aDataMember)
    {
        return get(*(mElement.*aDataMember));
    }

    template <class T_member>
    std::vector<Owned<T_member>> 
    iterate(std::vector<gltf::Index<T_member>> T_element::* aMemberIndexVector)
    {
        std::vector<Owned<T_member>> result;
        for (gltf::Index<T_member> index : mElement.*aMemberIndexVector)
        {
            result.emplace_back(mOwningGltf.get(index));
        }
        return result;
    }

    // TODO make lazy iteration
    template <class T_member>
    std::vector<Owned<T_member>> iterate(std::vector<T_member> T_element::* aMemberVector)
    {
        std::vector<Owned<T_member>> result;
        for (std::size_t id = 0; id != (mElement.*aMemberVector).size(); ++id)
        {
            result.emplace_back(mOwningGltf, (mElement.*aMemberVector)[id], id);
        }
        return result;
    }

    filesystem::path getFilePath(gltf::Uri aUri) const
    {
        return mOwningGltf.getPathFor(aUri);
    }

    filesystem::path getFilePath(gltf::Uri T_element::* aMemberUri) const
    {
        return mOwningGltf.getPathFor(mElement.*aMemberUri);
    }

    filesystem::path getFilePath(std::optional<gltf::Uri> T_element::* aMemberUri) const
    {
        return mOwningGltf.getPathFor(*(mElement.*aMemberUri));
    }


private:
    Gltf & mOwningGltf;
    T_element & mElement; 
    gltf::Index<T_element> mIndex;
};



template <class T_element>
class Const_Owned
{
    friend class Gltf;

public:
    /*implicit*/Const_Owned(Owned<T_element> aOwned);

    Const_Owned(const Gltf & aGltf, const T_element & aElement, gltf::Index<T_element> aIndex);

    operator const T_element &() const
    { return mElement; }

    const T_element * operator->() const
    {
        return &mElement;
    }

    const T_element & operator*() const
    {
        return mElement;
    }

    gltf::Index<T_element> id() const
    {
        return mIndex;
    }

    template <class T_indexed>
    Const_Owned<T_indexed> get(gltf::Index<T_indexed> aIndex) const;

    template <class T_indexed>
    Const_Owned<T_indexed> get(gltf::Index<T_indexed> T_element::* aDataMember) const
    {
        return get(mElement.*aDataMember);
    }

    template <class T_indexed>
    Const_Owned<T_indexed> get(std::optional<gltf::Index<T_indexed>> T_element::* aDataMember) const
    {
        return get(*(mElement.*aDataMember));
    }

    template <class T_member>
    std::vector<Const_Owned<T_member>> 
    iterate(std::vector<gltf::Index<T_member>> T_element::* aMemberIndexVector) const
    {
        std::vector<Const_Owned<T_member>> result;
        for (gltf::Index<T_member> index : mElement.*aMemberIndexVector)
        {
            result.emplace_back(mOwningGltf.get(index));
        }
        return result;
    }

    // TODO make lazy iteration
    template <class T_member>
    std::vector<Const_Owned<T_member>> iterate(std::vector<T_member> T_element::* aMemberVector) const
    {
        std::vector<Const_Owned<T_member>> result;
        for (std::size_t id = 0; id != (mElement.*aMemberVector).size(); ++id)
        {
            result.emplace_back(mOwningGltf, (mElement.*aMemberVector)[id], id);
        }
        return result;
    }

    filesystem::path getFilePath(gltf::Uri aUri) const
    {
        return mOwningGltf.getPathFor(aUri);
    }

    filesystem::path getFilePath(gltf::Uri T_element::* aMemberUri) const
    {
        return mOwningGltf.getPathFor(mElement.*aMemberUri);
    }

    filesystem::path getFilePath(std::optional<gltf::Uri> T_element::* aMemberUri) const
    {
        return mOwningGltf.getPathFor(*(mElement.*aMemberUri));
    }


private:
    const Gltf & mOwningGltf;
    const T_element & mElement; 
    const gltf::Index<T_element> mIndex;
};


//
// Implementations
//
namespace gltf 
{


    template <class T_indexed>
    std::ostream & operator<<(std::ostream & aOut, const std::vector<T_indexed> & aIndexVector)
    {
        auto begin = std::begin(aIndexVector);
        auto end = std::end(aIndexVector);

        aOut << "[";
        if (begin != end)
        {
            aOut << *begin;
            ++begin;
        }
        for (; begin != end; ++begin)
        {
            aOut << ", " << *begin;
        }
        return aOut << "]";
    }

     
} // namespace gltf


template <class T_element>
Owned<T_element>::Owned(Gltf & aGltf, T_element & aElement, gltf::Index<T_element> aIndex) :
    mOwningGltf{aGltf},
    mElement{aElement},
    mIndex{aIndex}
{}


template <class T_element>
template <class T_indexed>
Owned<T_indexed> Owned<T_element>::get(gltf::Index<T_indexed> aIndex)
{
    return mOwningGltf.get(aIndex);
}

template <class T_element>
Const_Owned<T_element>::Const_Owned(Owned<T_element> aOwned) :
    Const_Owned{aOwned.mOwningGltf, aOwned.mElement, aOwned.mIndex}
{}


template <class T_element>
Const_Owned<T_element>::Const_Owned(const Gltf & aGltf, const T_element & aElement, gltf::Index<T_element> aIndex) :
    mOwningGltf{aGltf},
    mElement{aElement},
    mIndex{aIndex}
{}


template <class T_element>
template <class T_indexed>
Const_Owned<T_indexed> Const_Owned<T_element>::get(gltf::Index<T_indexed> aIndex) const
{
    return mOwningGltf.get(aIndex);
}


} // namespace arte
} // namespace ad
