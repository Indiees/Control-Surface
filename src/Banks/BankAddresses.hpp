#pragma once

#include "BankableAddresses.hpp"

BEGIN_CS_NAMESPACE

namespace Bankable {

class SingleAddress : public OutputBankableMIDIAddress {
  public:
    SingleAddress(OutputBankConfig config, MIDIAddress address)
        : OutputBankableMIDIAddress{config}, address{address} {}

    MIDIAddress getBaseAddress() const { return address; }

    MIDIAddress getActiveAddress() const {
        return getBaseAddress() + getAddressOffset();
    }

  private:
    MIDIAddress address;
};

template <uint8_t N>
class SingleAddressMultipleBanks {
  public:
    SingleAddressMultipleBanks(const Array<OutputBankableMIDIAddress, N> &banks,
                               MIDIAddress address)
        : banks{banks}, address{address} {}

    MIDIAddress getBaseAddress() const { return address; }

    MIDIAddress getActiveAddress() const {
        auto address = getBaseAddress();
        for (const OutputBankableMIDIAddress &bank : banks)
            address += bank.getAddressOffset();
        return address;
    }

    void lock() {
        for (OutputBankableMIDIAddress &bank : banks)
            bank.lock();
    }

    void unlock() {
        for (OutputBankableMIDIAddress &bank : banks)
            bank.unlock();
    }

  private:
    Array<OutputBankableMIDIAddress, N> banks;
    MIDIAddress address;
};

class DualAddresses : public OutputBankableMIDIAddress {
  public:
    DualAddresses(OutputBankConfig config,
                  const Array<MIDIAddress, 2> &addresses)
        : OutputBankableMIDIAddress{config}, first(addresses[0]),
          second(addresses[1]) {}

    MIDIAddress getFirstBaseAddress() const { return first; }
    MIDIAddress getSecondBaseAddress() const { return second; }

    MIDIAddress getFirstActiveAddress() const {
        return getFirstBaseAddress() + getAddressOffset();
    }
    MIDIAddress getSecondActiveAddress() const {
        return getSecondBaseAddress() + getAddressOffset();
    }

  private:
    MIDIAddress first, second;
};

template <uint8_t nb_rows, uint8_t nb_cols>
class MatrixAddress : public OutputBankableMIDIAddress {
  public:
    MatrixAddress(OutputBankConfig config,
                  const AddressMatrix<nb_rows, nb_cols> &addresses,
                  MIDIChannelCN channelCN)
        : OutputBankableMIDIAddress{config}, addresses{addresses},
          channelCN{channelCN} {}

    uint8_t getBaseAddress(uint8_t row, uint8_t col) const {
        return addresses[row][col];
    }

    MIDIAddress getActiveAddress(uint8_t row, uint8_t col) const {
        MIDIAddress address = {getBaseAddress(row, col), channelCN};
        return address + getAddressOffset();
    }

  private:
    AddressMatrix<nb_rows, nb_cols> addresses;
    MIDIChannelCN channelCN;
};

namespace ManyAddresses {

/**
 * @tparam  NumBanks
 *          The number of bank settings the bank has.
 */
template <uint8_t NumBanks>
class ManyAddresses : public ManyAddresses_Base {
  public:
    /**
     * @brief   Constructor.
     * 
     * @param   bank
     *          The bank to add this element to.
     * @param   addresses
     *          The list of alternative addresses.
     */
    ManyAddresses(const Bank<NumBanks> &bank,
                  const Array<MIDIAddress, NumBanks> &addresses)
        : ManyAddresses_Base{bank}, addresses{addresses} {}

    MIDIAddress getActiveAddress() const { return addresses[getSelection()]; }

  private:
    Array<MIDIAddress, NumBanks> addresses;
};

/**
 * @tparam  NumBanks
 *          The number of bank settings the bank has.
 */
template <uint8_t NumBanks>
class DualManyAddresses : public ManyAddresses_Base {
  public:
    DualManyAddresses(const Bank<NumBanks> &bank,
                      const Array2D<MIDIAddress, 2, NumBanks> &addresses)
        : ManyAddresses_Base{bank}, first{addresses[0]}, second{addresses[1]} {}

    MIDIAddress getFirstActiveAddress() const { return first[getSelection()]; }
    MIDIAddress getSecondActiveAddress() const {
        return second[getSelection()];
    }

  private:
    Array<MIDIAddress, NumBanks> first, second;
};

template <uint8_t NumBanks, uint8_t nb_rows, uint8_t nb_cols>
class ManyMatrixAddresses : public ManyAddresses_Base {
  public:
    ManyMatrixAddresses(
        const Bank<NumBanks> &bank,
        const Array<AddressMatrix<nb_rows, nb_cols>, NumBanks> &addresses,
        const Array<MIDIChannelCN, NumBanks> &channelCNs)
        : ManyAddresses_Base{bank}, addresses{addresses}, channelCNs{
                                                              channelCNs} {}

    MIDIAddress getActiveAddress(uint8_t row, uint8_t col) const {
        return {addresses[getSelection()][row][col],
                channelCNs[getSelection()]};
    }

  private:
    Array<AddressMatrix<nb_rows, nb_cols>, NumBanks> addresses;
    Array<MIDIChannelCN, NumBanks> channelCNs;
};

} // namespace ManyAddresses
} // namespace Bankable

END_CS_NAMESPACE