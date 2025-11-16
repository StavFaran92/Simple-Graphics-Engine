#include "WorkingDirectory.h"

#include "core/Engine.h"

WorkingDirectory::WorkingDirectory()
    : m_path(Engine::get()->getSubSystem<FileSystem>()->getContentDirectory())
{
}

ContentPath WorkingDirectory::path() const
{
	return m_path;
}

void WorkingDirectory::back()
{
	auto filesystem = Engine::get()->getSubSystem<FileSystem>();
	if (FileSystem::toAbsolute(m_path.raw()) != FileSystem::toAbsolute(filesystem->getContentDirectory()))
	{
		m_path.setPath(m_path.raw().parent_path());
	}
}

void WorkingDirectory::enterFolder(const std::string& folder)
{
    fs::path candidate = m_path.raw() / folder;

    candidate = fs::weakly_canonical(FileSystem::toAbsolute(candidate));

    // Must exist and be directory
    if (!fs::exists(candidate) || !fs::is_directory(candidate))
        return;

    // Valid — update working dir
    m_path.setPath(candidate);
}

std::string WorkingDirectory::string() const
{
    return m_path.raw().generic_string();
}

fs::directory_iterator WorkingDirectory::iter() const
{
    return std::filesystem::directory_iterator(FileSystem::toAbsolute(m_path.raw()));
}