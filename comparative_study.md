# 📊 Comparative Study: Networking in R-Type

## 1. Transport Protocols

| Protocol          | Pros                                         | Cons                                        | Suitable for R-Type? |
|-------------------|----------------------------------------------|---------------------------------------------|-----------------------|
| **TCP**           | Reliable (no loss, ordered), easy to use     | High latency, blocking issues               | ❌ Too slow |
| **UDP (simple)**  | Very fast, low overhead                     | Unreliable (loss, out-of-order packets)     | ❌ Too risky |
| **UDP (Quake-style)** | Fast, adds minimal reliability (ack/seq), proven in games | More complex to implement | ✅ Yes |

---

## 2. Game Data Strategies

| Strategy              | Pros                                   | Cons                                  | Suitable for R-Type? |
|------------------------|----------------------------------------|---------------------------------------|-----------------------|
| **Full Snapshot**      | Very easy to implement                 | Huge bandwidth usage                   | ❌ Too heavy |
| **Delta Snapshot**     | Bandwidth-efficient, only send changes | More complex to implement              | ✅ Yes |
| **Raw Key States**     | Very simple (just pressed keys)        | Poor lag tolerance, hard to replay     | ❌ Not robust |
| **UserCmd Batching**   | Robust, lag-tolerant, enables prediction | Slightly more complex                  | ✅ Yes |

---

## 3. Networking Libraries

| Option             | Pros                                           | Cons                                | Suitable for R-Type? |
|--------------------|-----------------------------------------------|-------------------------------------|-----------------------|
| **POSIX Sockets**  | Full control, no dependency                   | Very low-level, verbose, error-prone | ❌ No |
| **Boost.Asio**     | Mature, robust, widely used                   | Heavy dependency, slow compilation   | ❌ Not ideal |
| **Asio Standalone**| Lightweight, header-only, async, cross-platform| Less popular than Boost              | ✅ Yes |
| **ENet**           | Provides built-in UDP reliability             | Black-box, less educational          | ❌ No |

---

## 4. Package Managers

| Manager          | Pros                                                                 | Cons                                                  | Suitable for R-Type? |
|------------------|----------------------------------------------------------------------|-------------------------------------------------------|-----------------------|
| **vcpkg**        | Cross-platform triplets, CMake toolchain, lockstep deps, easy onboarding | Requires toolchain setup, MSFT-centric docs           | ✅ **Chosen** |
| **Conan**        | Powerful dependency graphs, profiles, reproducible builds            | More concepts to learn, server/cache management       | ✅ Possible |
| **CPM.cmake**    | Super lightweight, fetches via CMake only                           | Fewer prebuilt binaries, relies on upstream CMake     | ✅ Possible (small projects) |
| **Manual (git submodules)** | Full control, zero external tooling                        | Maintenance burden, no version resolution             | ❌ No |
| **System pkg (apt/brew)** | Quick local installs                                        | Not portable across teammates/CI                      | ❌ No |


## 5. Conclusion

For a real-time multiplayer game like **R-Type**, low latency and efficient bandwidth usage are non-negotiable.  
After evaluating the alternatives, we adopt a stack that balances **speed**, **robustness**, and **implementation complexity**:

- **Transport:** Raw TCP is too slow for action gameplay, and “pure” UDP is too unreliable. **UDP with Quake-style reliability (sequence/ack numbers)** provides the right middle ground: fast delivery with enough signal to detect loss and recover gracefully.
- **Game Data:** Re-sending the whole world each tick (full snapshots) would quickly saturate the network. **Delta Snapshots** only transmit changes since the last acknowledged state, drastically reducing bandwidth. Pairing this with **UserCmd batching** on the client side improves lag tolerance and enables client-side prediction and reconciliation.
- **Networking Library:** **Asio Standalone** offers a clean, header-only, asynchronous API that compiles fast and works on all platforms without pulling the entire Boost ecosystem—ideal for both coursework and production-like prototypes.
- **Package Manager:** We choose **vcpkg** to ensure consistent cross-platform builds via the CMake toolchain file and triplets. This simplifies teammate onboarding and CI, while keeping dependencies (like Asio) reproducible across macOS, Linux, and Windows.

**Bottom line:**  
**UDP (Quake-style) + Delta Snapshots + UserCmd batching + Asio Standalone, managed with vcpkg** gives us the **best balance** of performance, reliability, portability, and learning value for the R-Type project.