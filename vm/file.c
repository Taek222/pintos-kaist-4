/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"
#include "userprog/process.h"
#include "threads/vaddr.h"

static bool file_backed_swap_in(struct page *page, void *kva);
static bool file_backed_swap_out(struct page *page);
static void file_backed_destroy(struct page *page);

/* DO NOT MODIFY this struct */
static const struct page_operations file_ops = {
	.swap_in = file_backed_swap_in,
	.swap_out = file_backed_swap_out,
	.destroy = file_backed_destroy,
	.type = VM_FILE,
};

/* The initializer of file vm */
void vm_file_init(void)
{
}

/* Initialize the file backed page */
bool file_backed_initializer(struct page *page, enum vm_type type, void *kva)
{
	struct uninit_page *uninit = &page->uninit;

	memset(uninit, 0, sizeof(struct uninit_page));

	/* Set up the handler */
	page->operations = &file_ops;
	struct file_page *file_page = &page->file;

	return true;
}

/* Swap in the page by read contents from the file. */
static bool
file_backed_swap_in(struct page *page, void *kva)
{
	struct file_page *file_page = &page->file;
}

/* Swap out the page by writeback contents to the file. */
static bool
file_backed_swap_out(struct page *page)
{
	struct file_page *file_page = &page->file;
}

/* Destory the file backed page. PAGE will be freed by the caller. */
static void
file_backed_destroy(struct page *page)
{
	struct file_page *file_page = &page->file;
}

/* Do the mmap */
void *
do_mmap(void *addr, size_t length, int writable, struct file *file, off_t offset)
{
	struct thread *curr = thread_current();

	struct mmap_file *mmap_file = (struct mmap_file *)malloc(sizeof(struct mmap_file));
	list_init(&mmap_file->page_list);
	list_push_back(&curr->mmap_list, &mmap_file->elem);

	mmap_file->file = file_reopen(file);
	if (mmap_file->file == NULL)
		return NULL;

	size_t read_bytes = length > file_length(mmap_file->file) ? file_length(mmap_file->file) : length;
	size_t zero_bytes = pg_round_up(read_bytes) - read_bytes;
	uintptr_t upage = addr;
	off_t ofs = offset;

	while (read_bytes > 0 || zero_bytes > 0)
	{
		size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
		size_t page_zero_bytes = PGSIZE - page_read_bytes;

		struct file_info *file_info = (struct file_info *)malloc(sizeof(struct file_info));

		file_info->file = mmap_file->file;
		file_info->ofs = ofs;
		file_info->page_read_bytes = page_read_bytes;
		file_info->page_zero_bytes = page_zero_bytes;

		if (!vm_alloc_page_with_initializer(VM_FILE, upage, writable, lazy_load_segment, file_info))
			return false;

		struct page *page = spt_find_page(&curr->spt, upage);
		if (page == NULL)
			return false;

		list_push_back(&mmap_file->page_list, &page->mmap_elem);

		read_bytes -= page_read_bytes;
		zero_bytes -= page_zero_bytes;
		upage += PGSIZE;
		ofs += PGSIZE;
	}

	return addr;
}

/* Do the munmap */
void do_munmap(void *addr)
{
}
