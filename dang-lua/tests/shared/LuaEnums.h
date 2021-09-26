#pragma once

#include "dang-lua/Types.h"

#include "catch2/catch.hpp"

CATCH_REGISTER_ENUM(dang::lua::Status,
                    dang::lua::Status::Ok,
                    dang::lua::Status::RuntimeError,
                    dang::lua::Status::MemoryError,
                    dang::lua::Status::MessageHandlerError,
                    dang::lua::Status::SyntaxError,
                    dang::lua::Status::Yield,
                    dang::lua::Status::FileError)

CATCH_REGISTER_ENUM(dang::lua::GCOption,
                    dang::lua::GCOption::Collect,
                    dang::lua::GCOption::Stop,
                    dang::lua::GCOption::Restart,
                    dang::lua::GCOption::Count,
                    dang::lua::GCOption::CountBytes,
                    dang::lua::GCOption::Step,
                    dang::lua::GCOption::IsRunning,
                    dang::lua::GCOption::Incremental,
                    dang::lua::GCOption::Generational)

CATCH_REGISTER_ENUM(dang::lua::Type,
                    dang::lua::Type::None,
                    dang::lua::Type::Nil,
                    dang::lua::Type::Boolean,
                    dang::lua::Type::LightUserdata,
                    dang::lua::Type::Number,
                    dang::lua::Type::String,
                    dang::lua::Type::Table,
                    dang::lua::Type::Function,
                    dang::lua::Type::Userdata,
                    dang::lua::Type::Thread)

CATCH_REGISTER_ENUM(dang::lua::ArithOp,
                    dang::lua::ArithOp::Add,
                    dang::lua::ArithOp::Sub,
                    dang::lua::ArithOp::Mul,
                    dang::lua::ArithOp::Mod,
                    dang::lua::ArithOp::Pow,
                    dang::lua::ArithOp::Div,
                    dang::lua::ArithOp::IDiv,
                    dang::lua::ArithOp::BinaryAnd,
                    dang::lua::ArithOp::BinaryOr,
                    dang::lua::ArithOp::BinaryXOr,
                    dang::lua::ArithOp::LeftShift,
                    dang::lua::ArithOp::RightShift,
                    dang::lua::ArithOp::UnaryMinus,
                    dang::lua::ArithOp::BinaryNot)

CATCH_REGISTER_ENUM(dang::lua::CompareOp,
                    dang::lua::CompareOp::Equal,
                    dang::lua::CompareOp::LessThan,
                    dang::lua::CompareOp::LessEqual)

CATCH_REGISTER_ENUM(dang::lua::Hook,
                    dang::lua::Hook::Call,
                    dang::lua::Hook::Ret,
                    dang::lua::Hook::Line,
                    dang::lua::Hook::Count,
                    dang::lua::Hook::TailCall)

CATCH_REGISTER_ENUM(dang::lua::DebugInfoType,
                    dang::lua::DebugInfoType::Name,
                    dang::lua::DebugInfoType::Source,
                    dang::lua::DebugInfoType::Line,
                    dang::lua::DebugInfoType::TailCall,
                    dang::lua::DebugInfoType::Upvalues)

CATCH_REGISTER_ENUM(dang::lua::StandardLibrary,
                    dang::lua::StandardLibrary::Base,
                    dang::lua::StandardLibrary::Coroutine,
                    dang::lua::StandardLibrary::Table,
                    dang::lua::StandardLibrary::IO,
                    dang::lua::StandardLibrary::OS,
                    dang::lua::StandardLibrary::String,
                    dang::lua::StandardLibrary::Utf8,
                    dang::lua::StandardLibrary::Math,
                    dang::lua::StandardLibrary::Debug,
                    dang::lua::StandardLibrary::Package)

CATCH_REGISTER_ENUM(dang::lua::LoadMode,
                    dang::lua::LoadMode::Default,
                    dang::lua::LoadMode::Binary,
                    dang::lua::LoadMode::Text,
                    dang::lua::LoadMode::Both)
