struct DeletedCopy { DeletedCopy()=default; DeletedCopy(const DeletedCopy&)=delete; };
static_assert(!__is_trivially_copyable(DeletedCopy),"false assertion must be rejected");
