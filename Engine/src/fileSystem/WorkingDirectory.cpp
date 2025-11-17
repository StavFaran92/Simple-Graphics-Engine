#include "WorkingDirectory.h"

#include "core/Engine.h"

WorkingDirectory::WorkingDirectory()
    : m_path(ScopedPath::ContentPath())
{
}

ScopedPath WorkingDirectory::path() const
{
	return m_path;
}

void WorkingDirectory::back()
{
	auto filesystem = Engine::get()->getSubSystem<FileSystem>();
    std::filesystem::path absoultePath = std::filesystem::weakly_canonical(FileSystem::toAbsolute(m_path.scoped()));
    std::filesystem::path absoultContentDir = std::filesystem::weakly_canonical(FileSystem::toAbsolute(filesystem->getContentDirectory()));
	if (absoultePath != absoultContentDir)
	{
		m_path.setPath(m_path.relative().parent_path());
	}
}

void WorkingDirectory::enterFolder(const std::string& folder)
{
    fs::path candidate = m_path.scoped() / folder;

    candidate = fs::weakly_canonical(FileSystem::toAbsolute(candidate));

    // Must exist and be directory
    if (!fs::exists(candidate) || !fs::is_directory(candidate))
        return;

    // Valid — update working dir
    m_path.setPath(candidate);
}

std::string WorkingDirectory::string() const
{
    return m_path.scoped().generic_string();
}

fs::directory_iterator WorkingDirectory::iter() const
{
    return std::filesystem::directory_iterator(FileSystem::toAbsolute(m_path.scoped()));
}