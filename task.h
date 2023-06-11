#pragma once

#include<iostream>
#include<coroutine>
#include<string>
#include<future>

namespace co_context {
	template<typename T>
	struct promise;

	template<typename T>
	struct coroutine : std::coroutine_handle<promise<T>> {
		using promise_type = co_context::promise<T>;
	};

	template<typename T>
	struct promise {
		std::future<T>future;//���첽
		std::string _str;
		int n;
		coroutine<T> get_return_object() { return { coroutine<T>::from_promise(*this) }; }
		std::suspend_never initial_suspend() noexcept { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }
		void return_value(const std::string& str) noexcept {
			_str = str;
			std::cout << str << '\n';
		}
		std::suspend_always yield_value(int n) {
			this->n = n;
			//puts("yield_value()������");
			return {};
		}
		void unhandled_exception() {}
	};
	template<>
	struct promise<void> {
		std::future<void>future;
		int n;
		coroutine<void> get_return_object() { return { coroutine<void>::from_promise(*this) }; }
		std::suspend_never initial_suspend() noexcept { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }
		void return_void() noexcept { /*puts("Э��ִ�����");*/ }
		std::suspend_always yield_value(int n) {
			this->n = n;
			//puts("yield_value()������");
			return {};
		}
		void unhandled_exception() {}
	};

	template<typename F, typename...Args>
	struct Input {
		using R = std::invoke_result_t<F, Args...>;

		F _f;
		std::tuple<Args...>_values;

		Input(F f, Args...args) :_f{ f }, _values{ args... } {}
		constexpr bool await_ready() const noexcept {
			return false;
		}
		constexpr void await_suspend(std::coroutine_handle<promise<R>>h)const noexcept {
			h.promise().future = std::async(std::launch::async,
				[this, h] {
					if constexpr (std::is_void_v<R>) {
						std::apply([&](Args...args) {return _f(args...); }, _values);
						if (!h.done())h.resume();//�ָ�Э��
					}
					else {
						R result = std::apply([&](Args...args) {return _f(args...); }, _values);
						if (!h.done())h.resume();//�ָ�Э��
						return result;
					}
				});//Ҫ�����첽����
			//puts("await_suspend ִ�����");
		}
		constexpr void await_resume()const noexcept {  }
	};
}