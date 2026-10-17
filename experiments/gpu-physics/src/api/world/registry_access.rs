//! Recursive public access must never wait on the lock owned by its callback.
//! Other threads still serialize normally; this is not a C-adapter thread-safety
//! promise. C metadata/combined wrappers check the same ownership before edits.
use super::*;
use std::{cell::Cell, ffi::c_char, ops::{Deref, DerefMut}, sync::MutexGuard};

thread_local! {
    static OWNED: Cell<bool> = const { Cell::new(false) };
    static ERROR: Cell<*const c_char> = const { Cell::new(std::ptr::null()) };
}
pub(super) fn allowed() -> bool { OWNED.with(|owned| !owned.get()) }
pub(super) fn error_name() -> *const c_char { ERROR.with(Cell::get) }
pub(super) fn clear_error() { ERROR.with(|error| error.set(std::ptr::null())); }
pub(super) fn record_error(name: *const c_char, code: i32) {
    ERROR.with(|error| error.set(name));
    #[cfg(target_os = "linux")]
    unsafe { *libc::__errno_location() = code; }
    #[cfg(not(target_os = "linux"))]
    let _ = code;
}
fn busy() { record_error(c"gpu_world_registry_reentry".as_ptr(), 16); } // EBUSY

pub(super) struct Guard { inner: MutexGuard<'static, WorldRegistry> }
impl Deref for Guard { type Target = WorldRegistry; fn deref(&self) -> &Self::Target { &self.inner } }
impl DerefMut for Guard { fn deref_mut(&mut self) -> &mut Self::Target { &mut self.inner } }
impl Drop for Guard { fn drop(&mut self) { OWNED.with(|owned| owned.set(false)); } }
pub(super) fn lock() -> Option<Guard> {
    if !allowed() { busy(); return None; }
    let inner = WORLDS.lock().unwrap_or_else(|e| e.into_inner());
    OWNED.with(|owned| owned.set(true));
    Some(Guard { inner })
}
pub(super) fn try_lock() -> Option<Guard> {
    if !allowed() { busy(); return None; }
    let inner = match WORLDS.try_lock() {
        Ok(inner) => inner,
        Err(std::sync::TryLockError::Poisoned(e)) => e.into_inner(),
        Err(std::sync::TryLockError::WouldBlock) => { busy(); return None; }
    };
    OWNED.with(|owned| owned.set(true));
    Some(Guard { inner })
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn recursive_access_is_busy_and_thread_local() {
        clear_error();
        let guard = lock().unwrap();
        assert!(!allowed());
        assert!(lock().is_none());
        assert_eq!(unsafe { std::ffi::CStr::from_ptr(error_name()) }.to_bytes(), b"gpu_world_registry_reentry");
        // Do not wait for another thread to acquire the owned registry.
        std::thread::spawn(|| { assert!(allowed()); assert!(error_name().is_null()); assert!(try_lock().is_none()); }).join().unwrap();
        drop(guard);
        assert!(allowed());
        clear_error(); assert!(error_name().is_null());
        drop(lock().unwrap());
    }
}
