# C++ 학습 시작점

먼저 `src/object_pool.hpp`에서 저장 구조를 정하고,
`src/object_pool.cpp`의 Acquire/Release TODO를 구현합니다.
C# 기존 구현 보완은 C++ 학습 다음 단계입니다.

## 설계 초안

풀은 객체를 소유하고 호출자는 포인터를 빌립니다.
반환 후 포인터 사용은 금지합니다. raw pointer만으로는 재획득 이후의
오래된 포인터를 구분할 수 없으므로 세대가 포함된 핸들은 후속 개선 과제입니다.
확장 시 주소가 안정적인 저장 구조를 선택하세요.
기본 테스트는 용량 2 이상, 중복 반환 시 std::invalid_argument 정책을 가정합니다.
고정/가변 용량 선택에 맞춰 저장 구조를 정할 수 있습니다.

## 준비 및 빌드

C++20 컴파일러와 CMake 3.20 이상, 공통 검증용 Node.js 22 이상이 필요합니다.
최초 CMake 구성은 고정 버전 nlohmann/json 3.11.3 다운로드를 위해 인터넷이 필요합니다.

```powershell
cmake -S solution -B solution/build
cmake --build solution/build --config Debug
ctest --test-dir solution/build -C Debug --output-on-failure
```

Visual Studio 생성기 사용 시 실행 파일은 solution/build/Debug/galaxy.exe입니다.
Ninja/MinGW 등 단일 구성 생성기에서는 solution/build/galaxy.exe입니다.

```powershell
'{"type":"init","participantId":"local"}' | .\solution\build\Debug\galaxy.exe --smoke
node scripts/verify-protocol.mjs -- .\solution\build\Debug\galaxy.exe
```

Linux CI 실행 파일은 solution/build/galaxy입니다.

## 테스트의 입력과 기대 결과

- Acquire → Release → Acquire: 동일 주소/슬롯, generation 1 → 2.
- 다른 속성으로 재획득: 이름·타입·색상·크기 모두 새 값, 활성 상태.
- 같은 객체 두 번 반환: 두 번째는 invalid_argument; 이후 두 획득은 서로 다른 주소.

--smoke는 init JSON 입력과 진입점만 검사합니다.
TODO가 남아 있는 동안 단위 테스트와 실제 프로토콜 검증은 실패하도록 되어 있습니다.
JSON 이벤트는 stdout, 로그는 stderr에만 출력합니다.

## 준비 시점 검증 결과

- 공통 npm run check: 12개 테스트와 예제 JSONL 검증 통과.
- C++ 빌드/진입점/단위 테스트/프로토콜 검증: 미실행.
  현재 PATH와 일반 설치 경로에서 컴파일러 및 CMake를 찾지 못했습니다.
- GitHub Actions: 파일만 준비했으며 원격 실행하지 않았습니다.
