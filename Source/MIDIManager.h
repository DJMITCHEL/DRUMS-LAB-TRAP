#pragma once
#include "Common.h"

namespace mdl
{
    /** Relaciona notas MIDI con pads. Cada pad tiene su propia nota (parametro padN_note);
        varias pads pueden compartir la misma nota. */
    class MIDIManager
    {
    public:
        void setNoteSource (int pad, const std::atomic<float>* noteParam) noexcept { notes[(size_t) pad] = noteParam; }

        /** Mascara de bits: bit i = el pad i responde a esta nota. */
        uint32_t padsForNote (int midiNote) const noexcept
        {
            uint32_t mask = 0;
            for (int i = 0; i < kNumPads; ++i)
                if (notes[(size_t) i] != nullptr && (int) std::lround (notes[(size_t) i]->load()) == midiNote)
                    mask |= (1u << i);
            return mask;
        }

    private:
        std::array<const std::atomic<float>*, kNumPads> notes {};
    };
}
