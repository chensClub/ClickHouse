#pragma once

#include <Storages/IStorage.h>
#include <Poco/URI.h>
#include <ext/shared_ptr_helper.h>


namespace DB
{
/**
 * This class represents table engine for external urls.
 * It sends HTTP GET to server when select is called and
 * HTTP POST when insert is called. In POST request the data is send
 * using Chunked transfer encoding, so server have to support it.
 */
class IStorageURLBase : public IStorage
{
public:
    Pipes read(
        const Names & column_names,
        const SelectQueryInfo & query_info,
        const Context & context,
        QueryProcessingStage::Enum processed_stage,
        size_t max_block_size,
        unsigned num_streams) override;

    BlockOutputStreamPtr write(const ASTPtr & query, const Context & context) override;

protected:
    IStorageURLBase(
        const Poco::URI & uri_,
        const Context & context_,
        const StorageID & id_,
        const String & format_name_,
        const ColumnsDescription & columns_,
        const ConstraintsDescription & constraints_,
        const String & compression_method_);

    Poco::URI uri;
    const Context & context_global;
    String compression_method;

private:
    String format_name;

    virtual std::string getReadMethod() const;

    virtual std::vector<std::pair<std::string, std::string>> getReadURIParams(
        const Names & column_names,
        const SelectQueryInfo & query_info,
        const Context & context,
        QueryProcessingStage::Enum & processed_stage,
        size_t max_block_size) const;

    virtual std::function<void(std::ostream &)> getReadPOSTDataCallback(
        const Names & column_names,
        const SelectQueryInfo & query_info,
        const Context & context,
        QueryProcessingStage::Enum & processed_stage,
        size_t max_block_size) const;

    virtual Block getHeaderBlock(const Names & column_names) const = 0;
};


class StorageURL : public ext::shared_ptr_helper<StorageURL>, public IStorageURLBase
{
    friend struct ext::shared_ptr_helper<StorageURL>;
public:
    StorageURL(
        const Poco::URI & uri_,
        const StorageID & table_id_,
        const String & format_name_,
        const ColumnsDescription & columns_,
        const ConstraintsDescription & constraints_,
        Context & context_,
        const String & compression_method_)
        : IStorageURLBase(uri_, context_, table_id_, format_name_, columns_, constraints_, compression_method_)
    {
    }

    String getName() const override
    {
        return "URL";
    }

    Block getHeaderBlock(const Names & /*column_names*/) const override
    {
        return getSampleBlock();
    }
};
}
